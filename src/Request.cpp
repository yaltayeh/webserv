#include "Request.hpp"
#include <iostream>

void Request::setMethod(std::string methodName)
{
}

void Request::setMethod(HttpMethod method)
{
}

Request::Request()
{
}

Request::~Request()
{
}

HttpMethod Request::getMethod()
{
	return method;
}

Request Request::parseRequest(std::string rawHttp)
{
	std::stringstream ss;

	ss << rawHttp;
	return Request::parseRequest(ss);
}

Request Request::parseRequest(std::istream &is)
{
	Request req;
	std::string method;
	std::string path;
	std::string http;

	is >> method >> path >> http;
	std::cout << "Method: " << method << std::endl;
	std::cout << "Path: " << method << std::endl;
	std::cout << "HTTP: " << method << std::endl;

	req.setMethod(Http::getHttpMethod(method));

	

	return req;
}
