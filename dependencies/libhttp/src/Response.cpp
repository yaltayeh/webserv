#include "Response.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <sstream>

namespace libhttp {

	Response::Response()
		: _statusCode(200), _headers(), _body() {}

	Response::Response(const Response& other)
		: _statusCode(other._statusCode), _headers(other._headers), _body(other._body) {}

	Response& Response::operator=(const Response& other) {
		if (this != &other) {
			_statusCode = other._statusCode;
			_headers = other._headers;
			_body = other._body;
		}
		return *this;
	}

	Response::~Response() {}

	// Setters
	void Response::setStatus(int statusCode) {
		_statusCode = statusCode;
	}
	void Response::setHeader(const std::string& key, const std::string& value) {
		_headers.insert(std::make_pair(key, value));
	}
	void Response::setBody(const std::string& body) {
		_body = body;
	}

	// Getters
	int Response::getStatus() const { return _statusCode; }
	std::multimap<std::string, std::string> Response::getHeaders() const { return _headers; }
	std::string Response::getBody() const { return _body; }

	// Helper to convert response to string
	std::string Response::toString() const {
		std::string response;
		response += HTTP_1_0;
		std::stringstream ss;
		ss << _statusCode;
		response += " " + ss.str() + " " + getStatusMessage(_statusCode) + "\r\n";
		for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			response += it->first + ": " + it->second + "\r\n";
		}
		response += "\r\n" + _body;
		return response;
	}

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Response &response)
{
	os << response.toString();
	return os;
}