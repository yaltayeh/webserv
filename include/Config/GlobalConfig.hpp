#pragma once

#include <vector>
#include "Config.hpp"
#include "HttpConfig.hpp"

struct GlobalConfig : public Config
{
	std::vector<struct HttpConfig> https;

	GlobalConfig() : Config() {};

	GlobalConfig(const GlobalConfig &other)
		: Config(other)
	{
		this->https = other.https;
	}

	GlobalConfig& operator=(const GlobalConfig &other)
	{
		if (this != &other)
		{
			Config::operator=(other);
			this->https = other.https;
		}
		return *this;
	}

	~GlobalConfig() {}
};