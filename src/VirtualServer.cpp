#include "VirtualServer.hpp"
#include "RequestRouter.hpp"
#include "RegEx.hpp"

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

int VirtualServer::checkRequest(const libhttp::Request &r)
{
	std::string host = r.getHeader("Host");
	std::vector<std::string>::iterator it;
	for (it = _config.server_names.begin();
	it != _config.server_names.end();
	it++)
	{
		if (RegEx::matchPattern(*it, host))
			return (1);
	}
	return 0;
}

libhttp::Response VirtualServer::handleRequest(const libhttp::Request &r)
{
	RequestRouter router;
	return router.route(r, _config);
}
