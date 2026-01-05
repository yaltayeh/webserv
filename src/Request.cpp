#include "Request.hpp"

namespace libhttp {

	Request::Request() {}

	Request::Request(const Request& other) {
		*this = other;
	}

	Request& Request::operator=(const Request& other) {
		if (this != &other) {
			_method = other._method;
			_uri = other._uri;
			_headers = other._headers;
			_body = other._body;
		}
		return *this;
	}

	Request::~Request() {}

}