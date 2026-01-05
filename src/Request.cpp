#include "../include/Request.hpp"

namespace libhttp {

	Request::Request() : _method(GET) {}

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


	// Setters
	void Request::setMethod(Method method) {
		_method = method;
	}
	void Request::setUri(const std::string& uri) {
		_uri = uri;
	}
	void Request::setHeader(const std::string& key, const std::string& value) {
		_headers.insert(std::make_pair(key, value));
	}
	void Request::setBody(const std::string& body) {
		_body = body;
	}

	// Getters
	Method Request::getMethod() const { return _method; }
	std::string Request::getUri() const { return _uri; }
	std::string Request::getHeader(const std::string& key) const {
		std::multimap<std::string, std::string>::const_iterator it = _headers.find(key);
		if (it != _headers.end()) {
			return it->second;
		}
		return "";
	}
	std::string Request::getBody() const { return _body; }
	std::multimap<std::string, std::string> Request::getHeaders() const { return _headers; }
}