#pragma once

#include "Config/ServerConfig.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/Response.hpp"

class VirtualServer
{
private:
	ServerConfig _config;

public:
	VirtualServer();
	VirtualServer(const ServerConfig &config);
	~VirtualServer();
	const ServerConfig &getConfig() const;
	int checkRequest(const libhttp::Request &r);
	libhttp::Response handleRequest(const libhttp::Request &r);
};
