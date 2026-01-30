#include "../include/Request.hpp"
#include "../include/Utils.hpp"
#include <sstream>
#include <unistd.h>
#include <cstdio> // For std::remove() (C++ standard library function)
#include "Request.hpp"

namespace libhttp
{

	Request::Request() : _method(GET) {}

	Request::Request(const Request &other)
	{
		*this = other;
	}

	Request &Request::operator=(const Request &other)
	{
		if (this != &other)
		{
			_method = other._method;
			_uri = other._uri;
			_headers = other._headers;
			_body = other._body;
		}
		return *this;
	}

	Request::~Request() {}

	// Setters
	void Request::setMethod(Method method)
	{
		_method = method;
	}
	void Request::setUri(const std::string &uri)
	{
		_uri = uri;
	}
	void Request::setHeader(const std::string &key, const std::string &value)
	{
		_headers.insert(std::make_pair(key, value));
	}
	void Request::setBody(const RequestBody &body)
	{
		_body = body;
	}

	// Getters
	Method Request::getMethod() const { return _method; }
	const std::string &Request::getUri() const { return _uri; }
	std::string Request::getHeader(const std::string &key) const
	{
		std::multimap<std::string, std::string>::const_iterator it = _headers.find(key);
		if (it != _headers.end())
		{
			return it->second;
		}
		return "";
	}
	const RequestBody &Request::getBody() const { return _body; }
	const std::multimap<std::string, std::string> &Request::getHeaders() const { return _headers; }

	// Helper to convert request to string
	std::string Request::toString() const
	{
		std::string response;
		response += methodToString(_method) + " " + _uri + " " + HTTP_1_0 + "\r\n";
		for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		{
			response += it->first + ": " + it->second + "\r\n";
		}
		response += "\r\n";
		if (_body.getContentLength() > 0)
			response += "[BODY DATA]";
		return response;
	}

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Request &request)
{
	os << request.toString();
	return os;
}