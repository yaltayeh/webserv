#include "routing/RouteUtils.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/ResponseBlender.hpp"
#include "libhttp/include/FileResponse.hpp"
#include "libhttp/include/Utils.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>

namespace routing
{
namespace utils
{

libhttp::Response *generateErrorResponse(int status_code,
                                         const LocationConfig &location)
{
	// Check for custom error page
	std::map<int, CommonConfig::ErrorPage>::const_iterator it =
		location.error_pages.find(status_code);

	if (it != location.error_pages.end())
	{
		const CommonConfig::ErrorPage &error_page = it->second;
		std::string error_file_path = location.root + error_page.uri;
		int response_status = status_code;
		if (error_page.customRes)
			response_status = error_page.response;

		// Try to serve custom error page
		if (fileExists(error_file_path))
		{
			const std::string &content_type = getMimeType(error_file_path, location);
			libhttp::FileResponse *fileResponse =
				new libhttp::FileResponse(error_file_path, content_type, response_status);
			return fileResponse;
		}
		else 
		{
			std::string content = error_page.uri;

			libhttp::ResponseBlender blender;
			
			blender.setStatus(response_status);

			blender.setHeader("Content-Type", "text/html");

			std::ostringstream ss;
			ss << content.size();
			blender.setHeader("Content-Length", ss.str());
			blender.setBody(content);
			libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
			return response;
		}
	}

	// Fall back to default error page
	libhttp::ResponseBlender blender(status_code);
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
	return response;
}

std::string resolvePath(const std::string &uri,
                        const std::string &location_path,
                        const LocationConfig &location)
{
	// First, normalize the URI to prevent path traversal attacks
	std::string normalized_uri = normalizePath(uri);
	
	// If normalization returns empty string, it's a path traversal attempt
	if (normalized_uri.empty() && !uri.empty())
	{
		return ""; // Signal path traversal attempt
	}
	
	// Check for alias directive
	if (!location.alias.empty())
	{
		// alias: strip location path from URI and prepend alias
		// Example: location /photos { alias /data/images; }
		// URI /photos/cat.jpg -> /data/images/cat.jpg

		std::string remaining_path = normalized_uri.substr(location_path.length());
		return location.alias + remaining_path;
	}

	// Use root directive (default behavior)
	// Example: root /var/www;
	// URI /images/cat.jpg -> /var/www/images/cat.jpg
	return location.root + normalized_uri;
}

std::string readFile(const std::string &path)
{
	std::ifstream file(path.c_str(), std::ios::binary);

	if (!file.is_open())
	{
		return "";
	}

	// Get file size
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read file into string
	std::string content;
	content.resize(size);

	if (!file.read(&content[0], size))
	{
		return "";
	}

	return content;
}

bool fileExists(const std::string &path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0;
}

bool isDirectory(const std::string &path)
{
	struct stat st;

	if (stat(path.c_str(), &st) != 0)
	{
		return false;
	}

	return S_ISDIR(st.st_mode);
}

std::string findIndexFile(const std::string &directory_path,
                          const LocationConfig &location)
{
	// Ensure directory path ends with /
	std::string dir = directory_path;
	if (!dir.empty() && dir[dir.length() - 1] != '/')
	{
		dir += "/";
	}

	// Check each index file in order
	for (size_t i = 0; i < location.index.size(); ++i)
	{
		std::string index_path = dir + location.index[i];

		if (fileExists(index_path) && !isDirectory(index_path))
		{
			return index_path;
		}
	}

	return ""; // No index file found
}

const std::string &getMimeType(const std::string &path,
                                const LocationConfig &location)
{
	size_t dotPos = path.rfind('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = path.substr(dotPos + 1);
		std::map<std::string, std::string>::const_iterator it = location.mimeTypes.find(extension);
		if (it != location.mimeTypes.end())
		{
			return it->second;
		}
	}
	return location.default_type; // Default MIME type
}

std::string normalizePath(const std::string &path)
{
	// Split path into segments
	std::vector<std::string> segments;
	std::vector<std::string> result;
	std::string current;
	
	for (size_t i = 0; i < path.length(); ++i)
	{
		char c = path[i];
		if (c == '/')
		{
			if (!current.empty())
			{
				segments.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += c;
		}
	}
	
	if (!current.empty())
	{
		segments.push_back(current);
	}
	
	// Process segments, handling . and ..
	for (size_t i = 0; i < segments.size(); ++i)
	{
		const std::string &seg = segments[i];
		
		if (seg == ".")
		{
			// Skip current directory references
			continue;
		}
		else if (seg == "..")
		{
			// Parent directory reference
			if (!result.empty())
			{
				result.pop_back();
			}
			else
			{
				// Trying to go above root - path traversal attempt!
				return "";
			}
		}
		else
		{
			result.push_back(seg);
		}
	}
	
	// Build normalized path
	std::string normalized = "/";
	for (size_t i = 0; i < result.size(); ++i)
	{
		if (i > 0)
		{
			normalized += "/";
		}
		normalized += result[i];
	}
	
	return normalized;
}

} // namespace utils
} // namespace routing
