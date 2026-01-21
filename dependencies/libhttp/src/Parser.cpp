#include "../include/Parser.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// TODO: Your server must be compatible with standard web browsers of your choice.
namespace libhttp {
	
	Parser::Parser(size_t maxBodySize, const std::string& tempPath) :
						_state(REQUEST_LINE),
						_buffer(""),
						_expectedBodyLength(0),
						_receivedBodyLength(0),
						_maxBodySize(maxBodySize),
						_tempPath(tempPath),
						_usesTempFile(false) {}

	Parser::~Parser() {
		if (_tempFile.is_open()) {
			_tempFile.close();
		}
		// Clean up temp file if we created one but request didn't complete
		// (If request completed, the Request object owns the file and will clean it up)
		if (_usesTempFile && !_tempFilePath.empty() && _state != COMPLETE) {
			unlink(_tempFilePath.c_str());
		}
	}
	
	//// //// parsing functions
	// int 0 -> success, 1 -> incomplete, -1 -> error
	State Parser::parse(const char* partial_request, size_t size) {
		if (_state == COMPLETE || _state == ERROR)
			return _state;

		_buffer.append(partial_request, size);

		if (_state == REQUEST_LINE) {
			if (processRequestLine() != 0)
				return ERROR;
		}

		if (_state == HEADERS) {
			if (processHeaders() != 0)
				return ERROR;
		}

		if (_state == BODY) {
			if (processBody() != 0)
				return ERROR;
		}

		return _state;
	}

	int Parser::processRequestLine() {
		size_t pos = _buffer.find("\r\n");
		if (pos != std::string::npos) {
			// GET /index.html HTTP/1.0
			std::string request_line = _buffer.substr(0, pos);
			_buffer.erase(0, pos + 2); // remove processed line

			// parse method, uri, version with spilt
			std::vector<std::string> request_line_parts = split(request_line, ' ');

			if (request_line_parts.size() != 3 
				// || request_line_parts[2] != HTTP_1_0
				)
				{ _state = ERROR; return -1; }
			_request.setMethod(stringToMethod(request_line_parts[0]));
			_request.setUri(request_line_parts[1]);

			if (_request.getMethod() == INVALID)
				{ _state = ERROR; return -1; }

			_state = HEADERS;
		}
		return 0;
	}

	int Parser::processHeaders() {
		size_t pos;

		while ((pos = _buffer.find("\r\n")) != std::string::npos) {
			if (pos == 0) {
				// End of headers
				_buffer.erase(0, 2); // remove the "\r\n"
				_state = BODY;
				return 0;
			}

			std::string header_line = _buffer.substr(0, pos);
			_buffer.erase(0, pos + 2); // remove processed line

			/**
			 * Split header on FIRST colon only
			 * 
			 * Headers can contain colons in the value (e.g., "Host: localhost:8080")
			 * So we must split manually on the first colon, not use split()
			 */
			size_t colon_pos = header_line.find(':');
			if (colon_pos == std::string::npos) {
				_state = ERROR;
				return -1;
			}

			std::string key = trim(header_line.substr(0, colon_pos));
			std::string value = trim(header_line.substr(colon_pos + 1));

			_request.setHeader(key, value);
			
			// Check for Content-Length header
			if (key == "Content-Length") {
				_expectedBodyLength = std::atol(value.c_str());
			}
		}
	
		return 0;
	}

	int Parser::processBody() {
		// If no Content-Length header, treat all remaining data as body
		if (_expectedBodyLength == 0) {
			_request.setBody(_buffer);
			_buffer.clear();
			_state = COMPLETE;
			return 0;
		}

		if (_maxBodySize )
		
		// Determine if we need to use temp file for this body
		// Use temp file if body will exceed max size OR if we already started using one
		if (!_usesTempFile && _expectedBodyLength > _maxBodySize) {
			_usesTempFile = true;
			createTempFile();
			if (_state == ERROR) {
				return -1;
			}
			
			// Write any body data we've already accumulated
			if (!_request.getBody().empty()) {
				writeTempFile(_request.getBody().c_str(), _request.getBody().size());
				_request.setBody(""); // Clear in-memory body
			}
		}
		
		// Accumulate body data
		size_t bytesToRead = _buffer.size();
		if (_receivedBodyLength + bytesToRead > _expectedBodyLength) {
			bytesToRead = _expectedBodyLength - _receivedBodyLength;
		}
		
		if (_usesTempFile) {
			// Write to temp file
			writeTempFile(_buffer.c_str(), bytesToRead);
			_buffer.erase(0, bytesToRead);
		} else {
			// Store in memory
			_request.setBody(_request.getBody() + _buffer.substr(0, bytesToRead));
			_buffer.erase(0, bytesToRead);
		}
		
		_receivedBodyLength += bytesToRead;
		
		// Check if we've received all expected body data
		if (_receivedBodyLength >= _expectedBodyLength) {
			if (_usesTempFile) {
				closeTempFile();
				_request.setBodyTempFile(_tempFilePath);
			}
			_state = COMPLETE;
		}
		
		return 0;
	}

	// Getters
	bool Parser::isComplete() const { return _state == COMPLETE; }
	bool Parser::isError() const { return _state == ERROR; }
	Request Parser::getRequest() const { return _request; }

	// Temp file helpers
	void Parser::createTempFile() {
		// Create temp directory if it doesn't exist
		struct stat st;
		if (stat(_tempPath.c_str(), &st) != 0) {
			mkdir(_tempPath.c_str(), 0755);
		}

		// Generate unique temp file name using PID, timestamp, and address
		// Using 'this' pointer address adds uniqueness for concurrent requests
		std::ostringstream oss;
		oss << _tempPath << "/body_" << getpid() << "_" << time(NULL) << "_" << this;
		_tempFilePath = oss.str();

		_tempFile.open(_tempFilePath.c_str(), std::ios::binary | std::ios::out);
		if (!_tempFile.is_open()) {
			std::cerr << "Failed to create temp file: " << _tempFilePath << std::endl;
			_state = ERROR;
		}
	}

	void Parser::writeTempFile(const char* data, size_t size) {
		if (_tempFile.is_open()) {
			_tempFile.write(data, size);
			if (!_tempFile.good()) {
				std::cerr << "Failed to write to temp file" << std::endl;
				_state = ERROR;
			}
		}
	}

	void Parser::closeTempFile() {
		if (_tempFile.is_open()) {
			_tempFile.flush();
			_tempFile.close();
		}
	}

}

std::ostream& operator<<(std::ostream& os, libhttp::State s) {
    switch (s) {
        case libhttp::REQUEST_LINE: return os << "REQUEST_LINE";
        case libhttp::HEADERS: return os << "HEADERS";
        case libhttp::BODY: return os << "BODY";
        case libhttp::COMPLETE: return os << "COMPLETE";
        case libhttp::ERROR: return os << "ERROR";
        case libhttp::CLOSED: return os << "CLOSED";
        case libhttp::BODY_EXCEEDED_MAX_SIZE: return os << "BODY_EXCEEDED_MAX_SIZE";
        default: return os << "UNKNOWN";
    }
}
