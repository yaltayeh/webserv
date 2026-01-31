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
						_body(maxBodySize, tempPath) {}

	Parser::~Parser() {}
	
	//// //// parsing functions
	// int 0 -> success, 1 -> incomplete, -1 -> error
	State Parser::parse(const char* partial_request, size_t size) {
		if (_state == COMPLETE || _state == ERROR || _state == BODY_EXCEEDED_MAX_SIZE 
			|| _state == ERROR_URI_TOO_LONG || _state == ERROR_HTTP_VERSION_NOT_SUPPORTED
			|| _state == ERROR_UNSUPPORTED_TRANSFER_ENCODING || _state == ERROR_HEADER_TOO_LARGE
			|| _state == ERROR_EMPTY_URI || _state == ERROR_INVALID_HEADER 
			|| _state == ERROR_LENGTH_REQUIRED)
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
		
		// Validate URI is not empty
		std::string uri = request_line_parts[1];
		if (uri.empty()) {
			_state = ERROR_EMPTY_URI;
			return -1;
		}
		
		// Validate URI length (before decoding) - RFC 2616 doesn't specify limit, 
		// but most servers use 8KB as reasonable limit
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
			_state = ERROR_INVALID_HEADER;
			return -1;
		}

		std::string key_raw = header_line.substr(0, colon_pos);
		
		// RFC 7230: Header field names must not have whitespace before the colon
		// Check if there's trailing whitespace in the key (before colon)
		if (!key_raw.empty() && (key_raw[key_raw.length() - 1] == ' ' 
			|| key_raw[key_raw.length() - 1] == '\t')) {
			_state = ERROR_INVALID_HEADER;
			return -1;
		}
		
		std::string key = trim(key_raw);
		std::string value = trim(header_line.substr(colon_pos + 1));
		
		// Header key cannot be empty
		if (key.empty()) {
			_state = ERROR_INVALID_HEADER;
			return -1;
		}

		// Validate header size (key + value should not exceed 8KB)
		if (header_line.length() > 8192) {
			_state = ERROR_HEADER_TOO_LARGE;
			return -1;
		}

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
		// Check for body without Content-Length
		// For POST/PUT requests, Content-Length is required if there's a body
		if (!_hasContentLength) {
			// If there's data in the buffer after headers, it's an error
			if (!_buffer.empty()) {
				_state = ERROR_LENGTH_REQUIRED;
				return -1;
			}
			// No body expected, request is complete
			_request.setBody(_body);
			_state = COMPLETE;
			return 0;
		}
		
		// If Content-Length is 0, there should be no body data
		if (_expectedBodyLength == 0) {
			// If there's data after headers when Content-Length is 0, it's an error
			if (!_buffer.empty()) {
				_state = ERROR;
				return -1;
			}
			_request.setBody(_body);
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

		ssize_t bytesStored = _body.storeData(_buffer.c_str(), bytesToRead);
		if (bytesStored < 0) {
			_state = ERROR;
			return -1;
		}

		_buffer.erase(0, bytesStored);
		_receivedBodyLength += bytesStored;
		
		// Check if we've received all expected body data
		if (_receivedBodyLength >= _expectedBodyLength) {
			_request.setBody(_body);
			_state = COMPLETE;
		}
		
		return 0;
	}

	// Getters
	bool Parser::isComplete() const { return _state == COMPLETE; }
	bool Parser::isError() const { return _state == ERROR; }
	const Request &Parser::getRequest() const { return _request; }
	const RequestBody &Parser::getBody() const { return _body; }
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
        case libhttp::ERROR_HEADER_TOO_LARGE: return os << "ERROR_HEADER_TOO_LARGE";
        case libhttp::ERROR_EMPTY_URI: return os << "ERROR_EMPTY_URI";
        case libhttp::ERROR_INVALID_HEADER: return os << "ERROR_INVALID_HEADER";
        case libhttp::ERROR_LENGTH_REQUIRED: return os << "ERROR_LENGTH_REQUIRED";
        default: return os << "UNKNOWN";
    }
}
