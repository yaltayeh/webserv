#pragma once

#include <vector>
#include <string>
#include <map>
#include "CommonConfig.hpp"
#include "ServerConfig.hpp"

struct HttpConfig : public CommonConfig
{
	std::vector<struct ServerConfig *> servers;

	HttpConfig()
		: CommonConfig() {};

	HttpConfig(const HttpConfig &other)
	{
		CommonConfig(varname);
		this->servers = other.servers;
	}
};
