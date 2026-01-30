#include "../include/Request.hpp"
#include "../include/Utils.hpp"
#include <sstream>
#include <unistd.h>
#include <cstdio>  // For std::remove() (C++ standard library function)

namespace libhttp {

	Request::Request() : _method(GET), _hasBodyTempFile(false) {}

	Request::Request(const Request& other) {
		*this = other;
	}

	Request& Request::operator=(const Request& other) {
		if (this != &other) {
			_method = other._method;
			_uri = other._uri;
			_headers = other._headers;
			_body = other._body;
			_bodyTempFile = other._bodyTempFile;
			_hasBodyTempFile = other._hasBodyTempFile;
		}
		return *this;
	}

	Request::~Request() {
		// Clean up temp file if it exists
		// Using std::remove() (C++ stdlib function, not unlink() syscall)
		if (_hasBodyTempFile && !_bodyTempFile.empty()) {
			std::remove(_bodyTempFile.c_str());
		}
	}


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
	void Request::setBodyTempFile(const std::string& tempFilePath) {
		_bodyTempFile = tempFilePath;
		_hasBodyTempFile = true;
	}

	// Getters
	Method Request::getMethod() const { return _method; }
	const std::string &Request::getUri() const { return _uri; }
	std::string Request::getHeader(const std::string& key) const {
		std::multimap<std::string, std::string>::const_iterator it = _headers.find(key);
		if (it != _headers.end()) {
			return it->second;
		}
		return "";
	}
	const std::string &Request::getBody() const { return _body; }
	const std::multimap<std::string, std::string> &Request::getHeaders() const { return _headers; }
	bool Request::hasBodyTempFile() const { return _hasBodyTempFile; }
	const std::string &Request::getBodyTempFile() const { return _bodyTempFile; }

	// Helper to convert request to string
	std::string Request::toString() const {
		std::string response;
		response += methodToString(_method) + " " + _uri + " " + HTTP_1_0 + "\r\n";
		for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			response += it->first + ": " + it->second + "\r\n";
		}
		response += "\r\n" + _body;
		return response;
	}

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Request &request)
{
	os << request.toString();
	return os;
}