#include <iostream>
#include <unistd.h>
#include <sstream>
#include "RouteHandler.hpp"
#include "CGI.hpp"

// STEP 7: Determine if CGI or static file
// if (isCGIRequest(uri, location))
// {
// 	return handleCGI(request, uri, location);
// }

/**
 * isCGIRequest() - Determines if request should be handled by CGI
 *
 * Checks:
 * - cgi_pass is configured
 * - File extension matches cgi_extensions list
 *
 * RETURNS:
 * - true if this should be handled as CGI
 * - false for static file serving
 */
bool isCGIRequest(const libhttp::Request &request,
				  const LocationConfig &loc)
{
	std::cout << "isCGIRequest" << std::endl;
	const std::string &uri = request.getUri();

	// Check if CGI is configured
	if (loc.cgi_pass.empty())
	{
		return false;
	}

	// Check if URI has CGI extension
	for (size_t i = 0; i < loc.cgi_extensions.size(); ++i)
	{
		const std::string &ext = loc.cgi_extensions[i];

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
 * handleCGI - Executes a CGI script and returns its output
 *
 * Handles:
 * - Determining script path from URI
 * - Setting up CGI environment variables
 * - Executing script with appropriate interpreter
 * - Parsing CGI output (headers + body)
 * - Timeout handling
 * - 500 if CGI execution fails
 */

static libhttp::Response &generateResponse(const libhttp::Request &request,
									const LocationConfig &loc)
{
	static libhttp::StaticResponse response;
	const std::string &uri = request.getUri();

	// Get location path
	std::string location_path = loc.path;

	// Resolve script path
	std::string script_path = RouteHandler::resolvePath(uri, location_path, loc);

	std::cout << "CGI: Executing script at " << script_path << std::endl;

	// Check script exists
	if (!RouteHandler::fileExists(script_path))
	{
		return RouteHandler::generateErrorResponse(404, loc);
	}

	// Setup CGI execution
	CGI cgi;
	std::string interpreter = loc.cgi_pass;
	std::string server_name = "localhost"; // TODO: Get from server config
	int server_port = 8080;				   // TODO: Get from server config

	CGI::SetupResult setup_result = cgi.setup(request, script_path, interpreter,
											  server_name, server_port);

	if (setup_result != CGI::SETUP_OK)
	{
		std::cerr << "CGI setup failed: "
				  << CGI::getSetupResultMessage(setup_result) << std::endl;
		return RouteHandler::generateErrorResponse(500, loc);
	}

	// Execute CGI script
	int socketFd = -1;

	pid_t pid = cgi.execute(socketFd);

	if (pid == -1)
	{
		std::cerr << "CGI execution failed: " << cgi.getError() << std::endl;
		return RouteHandler::generateErrorResponse(500, loc);
	}

	// Write request body to CGI script (for POST requests)
	std::string request_body = request.getBody();
	if (!request_body.empty() && socketFd != -1)
	{
		ssize_t written = write(socketFd, request_body.c_str(),
								request_body.size());
		if (written < 0)
		{
			std::cerr << "Failed to write to CGI script" << std::endl;
		}
	}

	// Read CGI output
	std::string cgi_output;
	char buffer[4096];
	ssize_t bytes_read;

	while ((bytes_read = read(socketFd, buffer, sizeof(buffer))) > 0)
	{
		cgi_output.append(buffer, bytes_read);
	}

	close(socketFd);

	// Wait for CGI script to finish
	int status;
	waitpid(pid, &status, 0);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		std::cerr << "CGI script exited with error" << std::endl;
		return RouteHandler::generateErrorResponse(500, loc);
	}

	// Parse CGI output
	std::map<std::string, std::string> cgi_headers;
	std::string cgi_body;

	if (!CGI::parseCGIOutput(cgi_output, cgi_headers, cgi_body))
	{
		std::cerr << "Failed to parse CGI output" << std::endl;
		return RouteHandler::generateErrorResponse(500, loc);
	}

	// Build response from CGI output
	response.setStatus(200);

	// Copy CGI headers to response
	for (std::map<std::string, std::string>::const_iterator it = cgi_headers.begin();
		 it != cgi_headers.end(); ++it)
	{
		response.setHeader(it->first, it->second);
	}

	// Set Content-Length if not already set by CGI
	if (cgi_headers.find("Content-Length") == cgi_headers.end())
	{
		std::ostringstream ss;
		ss << cgi_body.size();
		response.setHeader("Content-Length", ss.str());
	}

	response.setBody(cgi_body);

	return response;
}

DEFINE_NORMAL_ROUTE_HANDLER(isCGIRequest, generateResponse);