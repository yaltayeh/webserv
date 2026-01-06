#include "../include/Parser.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

namespace libhttp {

	Parser::Parser() : 	_state(REQUEST_LINE),
						_buffer(""),
						_expectedBodyLength(0),
						_receivedBodyLength(0) {}
	Parser::~Parser() {}
	
	//// //// parsing functions
	// int 0 -> success, 1 -> incomplete, -1 -> error
	int Parser::parse(const char* partial_request, size_t size) {
		if (_state == COMPLETE || _state == ERROR)
			return (_state == COMPLETE) ? 0 : -1;

		_buffer.append(partial_request, size);

		if (_state == REQUEST_LINE) {
			if (processRequestLine() != 0)
				return -1;
		}

		if (_state == HEADERS) {
			if (processHeaders() != 0)
				return -1;
		}

		if (_state == BODY) {
			if (processBody() != 0)
				return -1;
		}

		if (_state == COMPLETE)
			return 0; // completed
		return 1; // incomplete
	}

	int Parser::processRequestLine() {
		size_t pos = _buffer.find("\r\n");
		if (pos != std::string::npos) {
			// GET /index.html HTTP/1.0
			std::string request_line = _buffer.substr(0, pos);
			_buffer.erase(0, pos + 2); // remove processed line

			// parse method, uri, version with spilt
			std::vector<std::string> request_line_parts = split(request_line, ' ');

			if (request_line_parts.size() != 3 || request_line_parts[2] != HTTP_1_0)
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

			std::vector<std::string> header_parts = split(header_line, ':');
			if (header_parts.size() != 2) { _state = ERROR; return -1; }

			std::string key = trim(header_parts[0]);
			std::string value = trim(header_parts[1]);

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
		
		// Accumulate body data
		size_t bytesToRead = _buffer.size();
		if (_receivedBodyLength + bytesToRead > _expectedBodyLength) {
			bytesToRead = _expectedBodyLength - _receivedBodyLength;
		}
		
		_request.setBody(_request.getBody() + _buffer.substr(0, bytesToRead));
		_buffer.erase(0, bytesToRead);
		_receivedBodyLength += bytesToRead;
		
		// Check if we've received all expected body data
		if (_receivedBodyLength >= _expectedBodyLength) {
			_state = COMPLETE;
		}
		
		return 0;
	}

	// simple state checkers
	bool Parser::isComplete() const { return _state == COMPLETE; }
	bool Parser::isError() const { return _state == ERROR; }

	// get the parsed request
	Request Parser::getRequest() const { return _request; }

}