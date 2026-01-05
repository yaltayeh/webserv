#include "Request.hpp"
#include <iostream>

void Request::setMethod(std::string methodName)
{
	(void)methodName;
}

void Request::setMethod(HttpMethod method)
{
	this->method = method;
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
	std::istringstream ss(rawHttp);

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
	std::cout << "Path: " << path << std::endl;
	std::cout << "HTTP: " << http << std::endl;

	req.setMethod(Http::getHttpMethod(method));

	

	return req;
}
