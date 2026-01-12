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
		: CommonConfig(NULL) {};
	HttpConfig(struct Config *parent)
		: CommonConfig(parent) {};
};
