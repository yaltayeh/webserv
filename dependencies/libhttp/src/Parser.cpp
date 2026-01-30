#include "../include/Parser.hpp"
#include "../include/Utils.hpp"
#include <StringUtils.hpp>  // from libutils (utils:: namespace)
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
						_hasContentLength(false),
						_hasTransferEncoding(false),
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
		if (_state == COMPLETE || _state == ERROR || _state == BODY_EXCEEDED_MAX_SIZE 
			|| _state == ERROR_URI_TOO_LONG || _state == ERROR_HTTP_VERSION_NOT_SUPPORTED
			|| _state == ERROR_UNSUPPORTED_TRANSFER_ENCODING)
			return _state;

		_buffer.append(partial_request, size);

		if (_state == REQUEST_LINE) {
			if (processRequestLine() != 0)
				return _state; // Return current state (might be specific error)
		}

		if (_state == HEADERS) {
			if (processHeaders() != 0)
				return _state; // Return current state (might be specific error)
		}

		if (_state == BODY) {
			if (processBody() != 0)
				return _state; // Return current state (might be specific error)
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

		if (request_line_parts.size() != 3)
			{ _state = ERROR; return -1; }
		
		// Validate URI length (before decoding) - RFC 2616 doesn't specify limit, 
		// but most servers use 8KB as reasonable limit
		std::string uri = request_line_parts[1];
		if (uri.length() > 8192) {
			_state = ERROR_URI_TOO_LONG;
			return -1;
		}
		
		// Validate HTTP version
		std::string version = request_line_parts[2];
		if (version != HTTP_1_0 && version != HTTP_1_1) {
			_state = ERROR_HTTP_VERSION_NOT_SUPPORTED;
			return -1;
		}
		
		_request.setMethod(stringToMethod(request_line_parts[0]));
		
		// Decode URI - split path and query string first
		std::string path, query;
		size_t query_pos = uri.find('?');
		if (query_pos != std::string::npos) {
			path = uri.substr(0, query_pos);
			query = uri.substr(query_pos); // includes the '?'
		} else {
			path = uri;
		}
		
		// Decode only the path part, keep query string as-is
		std::string decoded_path = utils::urlDecode(path);
		_request.setUri(decoded_path + query);

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
				
				// Check for Transfer-Encoding header - we don't support it
				if (_hasTransferEncoding) {
					_state = ERROR_UNSUPPORTED_TRANSFER_ENCODING;
					return -1;
				}
				
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
				// RFC 7230: Multiple Content-Length headers with different values is an error
				if (_hasContentLength) {
					_state = ERROR;
					return -1;
				}
				
				// Validate Content-Length value
				// Must be a non-negative integer
				if (value.empty()) {
					_state = ERROR;
					return -1;
				}
				
				// Check for negative sign
				if (value[0] == '-') {
					_state = ERROR;
					return -1;
				}
				
				// Parse the value
				char* endptr;
				long length = std::strtol(value.c_str(), &endptr, 10);
				
				// Check if parsing failed (endptr should point to end of string)
				if (*endptr != '\0' || length < 0) {
					_state = ERROR;
					return -1;
				}
				
				_expectedBodyLength = static_cast<size_t>(length);
				_hasContentLength = true;
			}
			
			// Check for Transfer-Encoding header
			if (key == "Transfer-Encoding") {
				_hasTransferEncoding = true;
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

		// Check if body size exceeds the maximum allowed (if maxBodySize is not 0 = unlimited)
		if (_maxBodySize > 0 && _expectedBodyLength > _maxBodySize) {
			_state = BODY_EXCEEDED_MAX_SIZE;
			return 0;
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
	const Request &Parser::getRequest() const { return _request; }

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
        case libhttp::ERROR_URI_TOO_LONG: return os << "ERROR_URI_TOO_LONG";
        case libhttp::ERROR_HTTP_VERSION_NOT_SUPPORTED: return os << "ERROR_HTTP_VERSION_NOT_SUPPORTED";
        case libhttp::ERROR_UNSUPPORTED_TRANSFER_ENCODING: return os << "ERROR_UNSUPPORTED_TRANSFER_ENCODING";
        default: return os << "UNKNOWN";
    }
}
