#include "routing/RouteUtils.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/Utils.hpp"
#include <sstream>
#include <fstream>
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

		// Try to serve custom error page
		if (fileExists(error_file_path))
		{
			std::string content = readFile(error_file_path);

			libhttp::DefaultResponse *response = new libhttp::DefaultResponse();

			if (error_page.customRes)
				response->setStatus(error_page.response);
			else
				response->setStatus(status_code);

			response->setHeader("Content-Type", "text/html");

			std::ostringstream ss;
			ss << content.size();
			response->setHeader("Content-Length", ss.str());

			response->setBody(content);
			return response;
		}
	}

	// Fall back to default error page
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse(status_code);
	return response;
}

std::string resolvePath(const std::string &uri,
                        const std::string &location_path,
                        const LocationConfig &location)
{
	// Check for alias directive
	if (!location.alias.empty())
	{
		// alias: strip location path from URI and prepend alias
		// Example: location /photos { alias /data/images; }
		// URI /photos/cat.jpg -> /data/images/cat.jpg

		std::string remaining_path = uri.substr(location_path.length());
		return location.alias + remaining_path;
	}

	// Use root directive (default behavior)
	// Example: root /var/www;
	// URI /images/cat.jpg -> /var/www/images/cat.jpg
	return location.root + uri;
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

} // namespace utils
} // namespace routing
