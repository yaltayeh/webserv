#include <string>
#include <iostream>
#include <map>

#pragma once
/*
https://httpwg.org/specs/rfc9110.html
*/

class Response
{
private:
	int status;
	std::map<std::string, std::string> headers;
	std::string content;

public:
	Response();
	Response(std::string content);
	~Response();

	void setStatus(int status);
	int getStatus() const;

	void setContent(const std::string content);
	const std::string &getContent() const;

	const std::string getReasonPhrase() const;

	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#http_responses
	const std::string getResponse() const;
};

const std::ostream &operator<<(std::ostream &os, const Response &res);
