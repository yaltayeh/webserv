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

	LocationConfig(std::string path)
		: CommonConfigServerLocation(NULL), path(path) {};

	LocationConfig(struct Config *parent, std::string path)
		: CommonConfigServerLocation(parent), path(path) {};
};
