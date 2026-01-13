#pragma once

#include "Config/ServerConfig.hpp"

class VirtualServer
{
private:
	ServerConfig _config;

public:
	VirtualServer();
	VirtualServer(const ServerConfig &config);
	~VirtualServer();
	const ServerConfig &getConfig() const;
};
