#include "HttpHeaders.hpp"

HttpHeaders::HttpHeaders()
{
}

HttpHeaders::~HttpHeaders()
{
}

void HttpHeaders::add(const std::string &key, const std::string &value)
{
	headers.insert(std::make_pair(key, value));
}

bool HttpHeaders::get(const std::string &key, std::string &outValue) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it == headers.end())
		return (false);

	outValue = it->second;
	return (true);
}

bool HttpHeaders::exist(const std::string &key) const
{
	return (headers.find(key) != headers.end());
}

bool HttpHeaders::remove(const std::string &key)
{
	return (headers.erase(key));
}

const std::multimap<std::string, std::string> &HttpHeaders::getAll() const
{
	return (headers);
}
