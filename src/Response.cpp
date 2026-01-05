#include "Response.hpp"
#include <sstream>

Response::Response() : status(200),
					   content("")
{
}

Response::Response(std::string content) : status(200),
										  content(content)
{
}

Response::~Response()
{
}

void Response::setContent(std::string content)
{
	this->content = content;
}

const std::string &Response::getContent() const
{
	return (content);
}

void Response::setStatus(int status)
{
	this->status = status;
}

int Response::getStatus() const
{
	return (this->status);
}

const std::string Response::getReasonPhrase() const
{
	return (std::string(""));
}

const std::string Response::getResponse() const
{
	std::stringstream ss;
	size_t content_length;

	content_length = content.length();

	// start line
	ss << "HTTP/1.0 " << status << std::endl;
	ss << "Content-Type: text/html; charset=UTF-8" << std::endl;
	ss << "Content-Length: " << content_length << std::endl;
	ss << std::endl;
	ss << content << std::endl;
	return ss.str();
}

const std::ostream &operator<<(std::ostream &os, const Response &res)
{
	std::string response_string = res.getResponse();
	os << response_string;
	return (os);
}
