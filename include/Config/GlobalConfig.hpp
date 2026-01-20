#pragma once

#include <vector>
#include "Config.hpp"
#include "HttpConfig.hpp"

struct GlobalConfig : public Config
{
	bool init_http;
	HttpConfig http;

	GlobalConfig() : Config(), init_http(false) {};

	GlobalConfig(const GlobalConfig &other)
		: Config(other)
	{
		this->http = other.http;
		this->init_http = other.init_http;
	}

	GlobalConfig& operator=(const GlobalConfig &other)
	{
		if (this != &other)
		{
			Config::operator=(other);
			this->http = other.http;
			this->init_http = other.init_http;
		}
		return *this;
	}

	~GlobalConfig() {}
};