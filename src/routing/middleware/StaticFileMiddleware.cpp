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
#include "libhttp/include/FileResponse.hpp"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

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
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse();
	response->setAutoindexPage(directory_path, uri);
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

	std::cout << "staticFileHandler: " << uri << std::endl;

	// Get location path
	std::string location_path = location.path;

	// Resolve URI to filesystem path
	std::string file_path = utils::resolvePath(uri, location_path, location);

	std::cout << "Resolving URI '" << uri << "' to path: " << file_path << std::endl;

	// Check if path exists
	if (!utils::fileExists(file_path))
	{
		return utils::generateErrorResponse(404, location);
	}

	// Handle directory
	if (utils::isDirectory(file_path))
	{
		// Look for index file
		std::string index_file = utils::findIndexFile(file_path, location);

		std::cout << "index_file: " << index_file << std::endl;

		if (!index_file.empty())
		{
			file_path = index_file;
		}
		else if (location.autoindex)
		{
			// Generate directory listing
			std::cout << "Autoindex enabled, generating directory listing" << std::endl;
			return generateAutoindex(file_path, uri);
		}
		else
		{
			// No index file and autoindex disabled
			std::cout << "Autoindex disabled (autoindex=" << location.autoindex << ")" << std::endl;
			return utils::generateErrorResponse(403, location);
		}
	}

	// Serve regular file using FileResponse for efficiency
	int fd = open(file_path.c_str(), O_RDONLY);
	if (fd < 0)
	{
		return utils::generateErrorResponse(404, location);
	}

	// Get file size for Content-Length
	off_t file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	// Build FileResponse
	libhttp::FileResponse *response = new libhttp::FileResponse();
	response->setStatus(200);
	response->setHeader("Content-Type", utils::getMimeType(file_path, location));

	std::ostringstream ss;
	ss << file_size;
	response->setHeader("Content-Length", ss.str());

	response->setFd(fd);

	return response;
}

} // namespace middleware
} // namespace routing
