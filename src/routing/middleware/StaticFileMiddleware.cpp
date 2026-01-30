/**
 * StaticFileMiddleware.cpp - Serve static files and directory listings
 *
 * Middleware that serves static files from the filesystem.
 * Handles index files, autoindex (directory listings), and regular files.
 * This is the fallback handler - always returns a response (never NULL).
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "routing/RouteUtils.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/ResponseBlender.hpp"
#include "libhttp/include/FileResponse.hpp"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>  // For std::remove() (C++ standard library function)

namespace routing
{
namespace middleware
{

/**
 * generateAutoindex() - Generate HTML directory listing using libhttp
 */
static libhttp::Response *generateAutoindex(const std::string &directory_path,
                                             const std::string &uri)
{
	// Use libhttp's built-in autoindex generator
	libhttp::ResponseBlender blender = libhttp::ResponseBlender();
	blender.setAutoindexPage(directory_path, uri);
	
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
	return response;
}

/**
 * staticFileHandler() - Middleware to serve static files
 *
 * ALWAYS returns a Response* (never NULL) - this is the fallback handler.
 * Serves files, directory indexes, or autoindex listings.
 */
libhttp::Response *staticFileHandler(const libhttp::Request &request,
                                      const LocationConfig &location,
                                      RouteContext &context)
{
	(void)context; // Not used in this middleware

	const std::string &uri = request.getUri();
	libhttp::Method method = request.getMethod();

	// std::cout << "staticFileHandler: " << uri << std::endl;

	// Get location path
	std::string location_path = location.path;
	
	// Strip query string from URI for file resolution
	std::string uri_path = uri;
	size_t query_pos = uri_path.find('?');
	if (query_pos != std::string::npos)
	{
		uri_path = uri_path.substr(0, query_pos);
	}

	// Resolve URI to filesystem path
	std::string file_path = utils::resolvePath(uri_path, location_path, location);

	// std::cout << "Resolving URI '" << uri_path << "' to path: " << file_path << std::endl;

	// Handle DELETE method
	if (method == libhttp::DELETE)
	{
		// Check if file exists
		if (!utils::fileExists(file_path))
		{
			return utils::generateErrorResponse(404, location);
		}
		
		// Check if it's a directory (can't delete directories)
		if (utils::isDirectory(file_path))
		{
			return utils::generateErrorResponse(403, location);
		}
		
		// Delete the file using std::remove() (C++ stdlib function, not unlink() syscall)
		if (std::remove(file_path.c_str()) != 0)
		{
			// Failed to delete file (permission error, etc.)
			return utils::generateErrorResponse(500, location);
		}
		
		// Success - return 204 No Content
		libhttp::ResponseBlender blender = libhttp::ResponseBlender();
		blender.setStatus(204);
		blender.setHeader("Content-Length", "0");
		
		libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
		return response;
	}

	// Handle PUT method (file upload)
	if (method == libhttp::PUT)
	{
		// For now, return 501 Not Implemented
		// Full PUT implementation would write request body to file
		return utils::generateErrorResponse(501, location);
	}

	// Check if path exists (for GET requests)
	if (!utils::fileExists(file_path))
	{
		return utils::generateErrorResponse(404, location);
	}

	// Handle directory
	if (utils::isDirectory(file_path))
	{
		// Look for index file
		std::string index_file = utils::findIndexFile(file_path, location);

		// std::cout << "index_file: " << index_file << std::endl;

		if (!index_file.empty())
		{
			file_path = index_file;
		}
		else if (location.autoindex)
		{
			// Generate directory listing
			// std::cout << "Autoindex enabled, generating directory listing" << std::endl;
			return generateAutoindex(file_path, uri);
		}
		else
		{
			// No index file and autoindex disabled
			// std::cout << "Autoindex disabled (autoindex=" << location.autoindex << ")" << std::endl;
			return utils::generateErrorResponse(403, location);
		}
	}

	// Build FileResponse
	try {
		const std::string &content_type = utils::getMimeType(file_path, location);
		libhttp::FileResponse *response = new libhttp::FileResponse(file_path, content_type, 200, method);
		return response;
	}
	catch (const std::exception &e) {
		std::cerr << "Error FileResponse: " << e.what() << std::endl;
		return utils::generateErrorResponse(404, location);
	}
	
}

} // namespace middleware
} // namespace routing
