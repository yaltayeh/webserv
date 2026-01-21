#pragma once

#include "Config/ServerConfig.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/Response.hpp"
#include <vector>

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
	const libhttp::Response &handleRequest(const libhttp::Request &r) const;

	static const VirtualServer *findMatchServer(const std::vector<VirtualServer *> &servers,
												const libhttp::Request &request);

	const LocationConfig &findMatchingLocation(const std::string &uri) const;

};
