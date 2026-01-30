/**
 * UploadMiddleware.cpp - Handle file uploads via POST/PUT requests
 *
 * Middleware that handles file uploads when upload_enable is configured.
 * Writes request body to files in the upload_store directory.
 * Returns NULL to pass to next middleware if uploads are not enabled.
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "routing/RouteUtils.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/ResponseBlender.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace routing
{
namespace middleware
{

/**
 * generateUniqueFilename() - Generate unique filename for uploaded file
 * 
 * Format: upload_<timestamp>_<pid>_<counter>
 */
static std::string generateUniqueFilename()
{
	static unsigned int counter = 0;
	
	std::ostringstream oss;
	oss << "upload_";
	oss << std::time(NULL);
	oss << "_";
	oss << getpid();
	oss << "_";
	oss << counter;
	counter++;
	return oss.str();
}

/**
 * ensureDirectoryExists() - Create directory if it doesn't exist
 * 
 * RETURNS:
 * - true: Directory exists or was created successfully
 * - false: Failed to create directory
 */
static bool ensureDirectoryExists(const std::string &path, bool create)
{
	struct stat st;
	
	// Check if directory exists
	if (stat(path.c_str(), &st) == 0)
	{
		// Path exists, check if it's a directory
		return S_ISDIR(st.st_mode);
	}
	
	// Directory doesn't exist
	if (!create)
		return false;
	
	// Try to create it
	if (mkdir(path.c_str(), 0755) == 0)
		return true;
	
	// Failed to create
	return false;
}

/**
 * writeUploadFile() - Write request body to file in upload directory
 * 
 * PARAMETERS:
 * - upload_dir: Directory to write the file to
 * - body: Request body content
 * - filename: Optional filename (generates unique name if empty)
 * 
 * RETURNS:
 * - true: File written successfully
 * - false: Failed to write file
 */
static bool writeUploadFile(const std::string &upload_dir, 
                            libhttp::RequestBody &body,
                            const std::string &filename)
{
	std::string final_filename = filename.empty() ? generateUniqueFilename() : filename;
	std::string file_path = upload_dir + "/" + final_filename;
	
	// Open file for writing (binary mode to preserve all content)
	std::ofstream file(file_path.c_str(), std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file for writing: " << file_path << std::endl;
		return false;
	}
	
	// Write body content
	const size_t buffer_size = 8192;
	char buffer[buffer_size];
	while (true)
	{
		ssize_t bytes_read = body.getChunk(buffer, buffer_size);
		if (bytes_read < 0)
		{
			std::cerr << "Failed to read from request body" << std::endl;
			file.close();
			return false;
		}
		if (bytes_read == 0)
			break; // EOF
		
		file.write(buffer, bytes_read);
		if (file.fail())
		{
			std::cerr << "Failed to write to file: " << file_path << std::endl;
			file.close();
			return false;
		}
	}	
	file.close();
	return true;
}

/**
 * uploadHandler() - Middleware to handle file uploads
 *
 * BEHAVIOR:
 * - Returns NULL if upload_enable is not configured (passes to next middleware)
 * - Returns NULL if method is not POST or PUT
 * - Returns error response if upload directory is not configured or inaccessible
 * - Writes request body to file in upload_store directory
 * - Returns 201 Created on success
 */
libhttp::Response *uploadHandler(const libhttp::Request &request,
                                  const LocationConfig &location,
                                  RouteContext &context)
{
	(void)context; // Not used in this middleware
	
	libhttp::Method method = request.getMethod();
	
	// Only handle POST and PUT requests
	if (method != libhttp::POST && method != libhttp::PUT)
		return NULL;
	
	// Check if uploads are enabled for this location
	if (!location.upload_enable)
		return NULL; // Pass to next middleware
	
	// Check if upload_store is configured
	if (location.upload_store.empty())
	{
		std::cerr << "Upload enabled but upload_store not configured" << std::endl;
		return utils::generateErrorResponse(500, location);
	}
	
	// Ensure upload directory exists
	bool create_dir = location.create_full_put_path;
	if (!ensureDirectoryExists(location.upload_store, create_dir))
	{
		std::cerr << "Upload directory does not exist or cannot be created: " 
		          << location.upload_store << std::endl;
		// Return 500 if we couldn't create it, or 403 if it exists but isn't a directory
		return utils::generateErrorResponse(500, location);
	}
	
	// Get request body
	libhttp::RequestBody body = request.getBody();
	
	// Write file (empty body is allowed - some tests check this)
	std::string empty_filename = "";
	if (!writeUploadFile(location.upload_store, body, empty_filename))
	{
		std::cerr << "Failed to write upload file" << std::endl;
		return utils::generateErrorResponse(500, location);
	}
	
	// Success - return 201 Created
	libhttp::ResponseBlender blender = libhttp::ResponseBlender();
	blender.setStatus(201);
	blender.setHeader("Content-Length", "0");
	
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
	return response;
}

} // namespace middleware
} // namespace routing
