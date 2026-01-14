#pragma once

#include <vector>
#include <string>
#include <map>

#include "CommonConfig.hpp"

struct LocationConfig : public CommonConfigServerLocation
{
	// args
	std::string path;
	// limit_except;

	std::string alias;

	// CGI/FastCGI configuration
	std::string fastcgi_pass;

	std::string fastcgi_split_path_info;

	// Allowed HTTP methods (empty = all allowed)
	// std::set<std::string> allowed_methods;

	LocationConfig()
		: CommonConfigServerLocation(),
		  path("") {};

	LocationConfig(const LocationConfig &other)
		: CommonConfigServerLocation(other)
	{
		this->path = other.path;
		this->alias = other.alias;
		this->fastcgi_pass = other.fastcgi_pass;
		this->fastcgi_split_path_info = other.fastcgi_split_path_info;
	};

	LocationConfig& operator=(const LocationConfig &other)
	{
		if (this != &other)
		{
			CommonConfigServerLocation::operator=(other);
			this->path = other.path;
			this->alias = other.alias;
			this->fastcgi_pass = other.fastcgi_pass;
			this->fastcgi_split_path_info = other.fastcgi_split_path_info;
		}
		return *this;
	}

	~LocationConfig() {}
};
