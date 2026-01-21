#include <iostream>
#include "VirtualServer.hpp"
#include "RegEx.hpp"
#include "DefaultResponse.hpp"
#include "routing/RouteHandler.hpp"

VirtualServer::VirtualServer() { }

VirtualServer::VirtualServer(const ServerConfig &config)
	: _config(config) { }

VirtualServer::~VirtualServer() { }

const ServerConfig &VirtualServer::getConfig() const
{
	return _config;
}

int VirtualServer::checkRequest(const libhttp::Request &r)
{
	std::string host_with_port = r.getHeader("Host");
	std::string host = host_with_port.find(':') != std::string::npos ?
				host_with_port.substr(0, host_with_port.find(':')) :
				host_with_port;

	std::vector<std::string>::iterator it;
	for (it = _config.server_names.begin();
		 it != _config.server_names.end();
		 it++)
	{
		std::cout << *it << " = " <<  host << std::endl;
		if (RegEx::matchPattern(*it, host))
			return (1);
	}
	return 0;
}

libhttp::Response *VirtualServer::handleRequest(const libhttp::Request &request) const
{
	const std::string &uri = request.getUri();
	const LocationConfig &location = findMatchingLocation(uri);

	// Use the new RouteHandler singleton to process the request
	return routing::RouteHandler::getInstance().process(request, location);
}

const VirtualServer *VirtualServer::findMatchServer(const std::vector<VirtualServer *> &servers,
													const libhttp::Request &request)
{
	if (servers.empty())
		return NULL;

	std::vector<VirtualServer *>::const_iterator it;
	for (it = servers.begin();
		 it != servers.end();
		 it++)
	{
		// check server_name is equal host
		if ((*it)->checkRequest(request))
			return *it;
	}
	return servers[0];
}

/**
 * findMatchingLocation() - Finds the best matching location block for a URI
 *
 * Implements longest-prefix matching:
 * - /images/cat.jpg matches both "/" and "/images"
 * - "/images" wins because it's more specific (longer prefix)
 * 
 * One-line rule to remember
 *  Exact → longest prefix → regex → prefix fallback
 * 
 * 
 * 
 * Choice algo 
 * First: exact match =
 * Second: longest prefix match (strings) // nginx remembers this match, but does NOT finalize yet.
 * ^~ prefix shortcut (IMPORTANT)
 * Third: regex locations (~ and ~*)
 * 
 * Final fallback
 *		If:
 *			no regex matched
 *			nginx falls back to:
 *			the best prefix match from step 2
 *
 * RETURNS:
 * - Pointer to matching LocationConfig, or NULL if no match
 */
const LocationConfig &VirtualServer::findMatchingLocation(const std::string &uri) const
{
	const LocationConfig *best_match;
	size_t longest_match = 0;

	// Iterate through all location blocks
	for (size_t i = 0; i < _config.locations.size(); ++i)
	{
		const LocationConfig *loc = &_config.locations[i];
		const std::string &loc_path = loc->path;

		// Check if URI starts with location path
		if (uri.compare(0, loc_path.length(), loc_path) == 0)
		{
			// This location matches, check if it's more specific
			if (loc_path.length() > longest_match)
			{
				longest_match = loc_path.length();
				best_match = loc;
			}
		}
	}

	return *best_match;
}
