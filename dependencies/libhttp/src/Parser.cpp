#include "../include/Parser.hpp"
#include <iostream>
namespace libhttp {

	Parser::Parser() : 	_state(REQUEST_LINE),
						_buffer(""),
						_request() {}
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
			_state = COMPLETE;
		}

		if (_state == COMPLETE)
			return 0; // completed
		return 1; // incomplete
	}

	int Parser::processRequestLine() {
		// Implementation for processing the request line
		return 0;
	}

	int Parser::processHeaders() {
		// Implementation for processing headers
		return 0;
	}

	int Parser::processBody() {
		// Implementation for processing body
		return 0;
	}
	
	// simple state checkers
	bool Parser::isComplete() const { return _state == COMPLETE; }
	bool Parser::isError() const { return _state == ERROR; }

	// get the parsed request
	Request Parser::getRequest() const { return _request; }

}