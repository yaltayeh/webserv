#include "StaticResponse.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <sstream>

namespace libhttp {

	StaticResponse::StaticResponse()
		: _statusCode(200), _headers(), _body() {}

	StaticResponse::StaticResponse(const StaticResponse& other)
		: _statusCode(other._statusCode), _headers(other._headers), _body(other._body) {}

	StaticResponse& StaticResponse::operator=(const StaticResponse& other) {
		if (this != &other) {
			_statusCode = other._statusCode;
			_headers = other._headers;
			_body = other._body;
		}
		return *this;
	}

	StaticResponse::~StaticResponse() {}

	// Setters
	void StaticResponse::setStatus(int statusCode) {
		_statusCode = statusCode;
	}
	void StaticResponse::setHeader(const std::string& key, const std::string& value) {
		_headers.insert(std::make_pair(key, value));
	}
	void StaticResponse::setBody(const std::string& body) {
		_body = body;
	}

	// Getters
	int StaticResponse::getStatus() const { return _statusCode; }
	std::multimap<std::string, std::string> StaticResponse::getHeaders() const { return _headers; }
	std::string StaticResponse::getBody() const { return _body; }

	// Helper to convert response to string
	std::string StaticResponse::toString() const {
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
std::ostream &operator<<(std::ostream &os, const libhttp::StaticResponse &response)
{
	os << response.toString();
	return os;
}