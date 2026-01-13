#include "VirtualServer.hpp"

VirtualServer::VirtualServer()
{
}

VirtualServer::VirtualServer(const ServerConfig &config)
	: _config(config)
{
}

VirtualServer::~VirtualServer()
{
}

const ServerConfig &VirtualServer::getConfig() const
{
	return _config;
}
