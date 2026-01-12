#pragma once

#include <vector>
#include <string>
#include <map>
#include "CommonConfig.hpp"
#include "LocationConfig.hpp"

struct ServerConfig : public CommonConfigServerLocation
{
	// Listen directives: address:port pairs
	std::vector<std::string> listen;

	// Server names (virtual host)
	std::vector<std::string> server_names;

	ServerConfig()
		: CommonConfigServerLocation(NULL),
		  listen(),
		  server_names()
	{
		listen.push_back("*:80");
		listen.push_back("*:8080");
	};
	ServerConfig(struct Config *parent) : CommonConfigServerLocation(parent) {};
};
