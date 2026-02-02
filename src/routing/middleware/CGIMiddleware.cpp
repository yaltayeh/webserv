/**
 * CGIMiddleware.cpp - Execute CGI scripts
 *
 * Middleware that detects CGI requests and executes them via libhttp::CGI.
 * Returns StreamResponse for non-blocking CGI execution.
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "routing/RouteUtils.hpp"
#include "libhttp/include/Request.hpp"
#include "StreamResponse.hpp"
#include "libhttp/include/CGI.hpp"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "libutils/include/Logger.hpp"
#include "libhttp/include/Utils.hpp"

namespace routing
{
namespace middleware
{

/**
 * isCGIRequest() - Check if request should be handled by CGI
 */
static bool isCGIRequest(const std::string &uri, const LocationConfig &location)
{
	// Check if CGI is configured
	if (location.cgi_handlers.empty())
		return false;

	// Strip query string from URI for extension checking
	std::string uri_path = uri;
	size_t query_pos = uri_path.find('?');
	if (query_pos != std::string::npos)
	{
		uri_path = uri_path.substr(0, query_pos);
	}

	// Check if URI ends with any registered CGI extension
	for (std::map<std::string, std::string>::const_iterator it = location.cgi_handlers.begin();
	     it != location.cgi_handlers.end(); ++it)
	{
		const std::string &ext = it->first;
		
		// Check if URI ends with this extension
		if (uri_path.length() >= ext.length() &&
		    uri_path.compare(uri_path.length() - ext.length(), ext.length(), ext) == 0)
		{
			return true;
		}
	}

	return false;
}

/**
 * cgiHandler() - Middleware to execute CGI scripts
 *
 * RETURNS:
 * - Response* (StreamResponse) if CGI handled
 * - Response* (error) if CGI fails
 * - NULL to continue to next middleware
 */
libhttp::Response *cgiHandler(const libhttp::Request &request,
                               const LocationConfig &location,
                               RouteContext &context)
{
	const std::string &uri = request.getUri();

	// Check if this is a CGI request
	if (!isCGIRequest(uri, location))
	{
		return NULL;
	}

	LOG_DEBUG("[CGIMiddleware] Handling CGI request: " 
	          << libhttp::methodToString(request.getMethod()) << " " << uri);

	// Strip query string from URI for path resolution
	std::string uri_path = uri;
	size_t query_pos = uri_path.find('?');
	if (query_pos != std::string::npos)
	{
		uri_path = uri_path.substr(0, query_pos);
	}

	// Get location path
	std::string location_path = location.path;

	// Resolve script path
	std::string script_path = utils::resolvePath(uri_path, location_path, location);

	LOG_DEBUG("CGI: Resolved script path: " << script_path);

	// Check script exists
	if (!utils::fileExists(script_path))
	{
		return utils::generateErrorResponse(404, location);
	}

	// Extract extension from URI
	std::string extension;
	size_t dot_pos = uri_path.rfind('.');
	if (dot_pos != std::string::npos)
		extension = uri_path.substr(dot_pos);  // Includes the '.'

	// Find interpreter for this extension
	std::map<std::string, std::string>::const_iterator handler_it = 
		location.cgi_handlers.find(extension);

	if (handler_it == location.cgi_handlers.end())
	{
		// Extension not registered (shouldn't happen if isCGIRequest passed)
		std::cerr << "CGI ERROR: No handler configured for extension " 
		          << extension << std::endl;
		return utils::generateErrorResponse(500, location);
	}

	std::string interpreter = handler_it->second;  // May be empty for binaries

	// Setup CGI execution
	CGI cgi;
	
	std::string server_name = context.get("server_name");
	if (server_name.empty())
		server_name = "localhost";

	std::string server_port_str = context.get("server_port");
	int server_port = server_port_str.empty() ? 8080 : std::atoi(server_port_str.c_str());

	CGI::SetupResult setup_result = cgi.setup(request, script_path, interpreter,
	                                          server_name, server_port);

	if (setup_result != CGI::SETUP_OK)
	{
		LOG_ERROR("CGI setup failed: "
		          << CGI::getSetupResultMessage(setup_result));
		return utils::generateErrorResponse(500, location);
	}

	// Execute CGI script
	int readPipeFd = -1;
	int writePipeFd = -1;
	pid_t pid = cgi.execute(readPipeFd, writePipeFd);

	if (pid == -1 || readPipeFd == -1 || writePipeFd == -1)
	{
		LOG_ERROR("CGI execution failed for script: " << script_path);
		return utils::generateErrorResponse(500, location);
	}

	// Create StreamResponse and set request body for async writing
	StreamResponse *response = new StreamResponse(readPipeFd, writePipeFd, pid, request.getBody());
	
	return response;
}

} // namespace middleware
} // namespace routing
