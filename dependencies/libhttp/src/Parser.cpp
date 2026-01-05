#include "../include/Parser.hpp"

namespace libhttp {

	Parser::Parser() : 	_state(REQUEST_LINE),
						_buffer(""),
						_request() {}
	Parser::~Parser() {}

	bool Parser::parse(const char* data, size_t size) {}
	
	// simple state checkers
	bool Parser::isComplete() const { return _state == COMPLETE; }
	bool Parser::isError() const { return _state == ERROR; }

	// get the parsed request
	Request Parser::getRequest() const { return _request; }

}