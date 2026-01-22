#pragma once

#include <vector>
#include <string>
#include <map>
#include "CommonConfig.hpp"
#include "ServerConfig.hpp"

struct HttpConfig : public CommonConfig
{
	std::vector<struct ServerConfig> servers;
	long idle_timeout; // in seconds, 0 = disabled

	HttpConfig()
		: CommonConfig(), idle_timeout(60 * 60) {}

	HttpConfig(const HttpConfig &other)
		: CommonConfig(other)
	{
		this->servers = other.servers;
		this->idle_timeout = other.idle_timeout;
	}

	HttpConfig& operator=(const HttpConfig &other)
	{
		if (this != &other)
		{
			CommonConfig::operator=(other);
			this->servers = other.servers;
			this->idle_timeout = other.idle_timeout;
		}
		return *this;
	}

	~HttpConfig() {}
};
