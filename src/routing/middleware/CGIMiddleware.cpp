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
	if (location.cgi_pass.empty())
		return false;

	// Check if URI has CGI extension
	for (size_t i = 0; i < location.cgi_extensions.size(); ++i)
	{
		const std::string &ext = location.cgi_extensions[i];

		// Check if URI ends with this extension
		if (uri.length() >= ext.length())
		{
			if (uri.compare(uri.length() - ext.length(), ext.length(), ext) == 0)
			{
				return true;
			}
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
		return NULL;

	// Get location path
	std::string location_path = location.path;

	// Resolve script path
	// HACK: If cgi_pass is a directory, use it as the CGI root instead
	std::string script_path;
	
	if (!location.cgi_pass.empty() && utils::isDirectory(location.cgi_pass))
	{
		// cgi_pass is being misused as a root directory
		// Strip location path from URI and append to cgi_pass
		
		// Safety check: ensure uri is long enough
		if (uri.length() < location_path.length())
		{
			std::cerr << "CGI ERROR: URI shorter than location path" << std::endl;
			return utils::generateErrorResponse(500, location);
		}
		
		std::string remaining = uri.substr(location_path.length());
		
		// Add separator if remaining doesn't start with /
		if (!remaining.empty() && remaining[0] != '/')
		{
			script_path = location.cgi_pass + "/" + remaining;
		}
		else
		{
			script_path = location.cgi_pass + remaining;
		}
	}
	else
	{
		script_path = utils::resolvePath(uri, location_path, location);
	}

	std::cout << "CGI: Executing script at " << script_path << std::endl;

	// Check script exists
	if (!utils::fileExists(script_path))
	{
		return utils::generateErrorResponse(404, location);
	}

	// Setup CGI execution
	CGI cgi;
	std::string interpreter = location.cgi_pass;
	
	// If cgi_pass looks like a directory, use default Python interpreter
	if (interpreter.find('/') != std::string::npos && utils::isDirectory(interpreter))
	{
		std::cout << "CGI: cgi_pass is directory, using /usr/bin/python3" << std::endl;
		interpreter = "/usr/bin/python3";
	}
	
	std::string server_name = context.get("server_name");
	if (server_name.empty())
		server_name = "localhost";

	std::string server_port_str = context.get("server_port");
	int server_port = server_port_str.empty() ? 8080 : atoi(server_port_str.c_str());

	CGI::SetupResult setup_result = cgi.setup(request, script_path, interpreter,
	                                          server_name, server_port);

	if (setup_result != CGI::SETUP_OK)
	{
		std::cerr << "CGI setup failed: "
		          << CGI::getSetupResultMessage(setup_result) << std::endl;
		return utils::generateErrorResponse(500, location);
	}

	// Execute CGI script
	int socketFd = -1;
	pid_t pid = cgi.execute(socketFd);

	if (pid == -1 || socketFd == -1)
	{
		std::cerr << "CGI execution failed: " << cgi.getError() << std::endl;
		return utils::generateErrorResponse(500, location);
	}

	// Write request body to CGI script (for POST requests)
	const std::string &request_body = request.getBody();
	if (!request_body.empty())
	{
		ssize_t written = write(socketFd, request_body.c_str(),
		                        request_body.size());
		if (written < 0)
		{
			std::cerr << "Failed to write to CGI script" << std::endl;
		}
	}

	// Return StreamResponse (non-blocking)
	return new StreamResponse(socketFd, pid);
}

} // namespace middleware
} // namespace routing
