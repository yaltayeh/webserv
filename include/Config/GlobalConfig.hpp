#pragma once

#include <vector>
#include "Config.hpp"
#include "HttpConfig.hpp"

struct GlobalConfig : public Config
{
	std::vector<struct HttpConfig *> https;

	GlobalConfig() : Config() {};
};