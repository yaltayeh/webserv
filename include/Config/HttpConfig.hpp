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
	int worker_connections; // maximum number of simultaneous connections

	HttpConfig()
		: CommonConfig(), idle_timeout(60 * 60), worker_connections(1024) {}

	HttpConfig(const HttpConfig &other)
		: CommonConfig(other)
	{
		this->servers = other.servers;
		this->idle_timeout = other.idle_timeout;
		this->worker_connections = other.worker_connections;
	}

	HttpConfig& operator=(const HttpConfig &other)
	{
		if (this != &other)
		{
			CommonConfig::operator=(other);
			this->servers = other.servers;
			this->idle_timeout = other.idle_timeout;
			this->worker_connections = other.worker_connections;
		}
		return *this;
	}

	~HttpConfig() {}
};
