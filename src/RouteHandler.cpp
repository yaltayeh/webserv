#include "RouteHandler.hpp"

#include "libhttp/include/Utils.hpp"
#include "libhttp/include/StaticResponse.hpp"
#include "libutils/include/StringUtils.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>

std::vector<struct Route> RouteHandler::permissionRoutes;
std::vector<struct Route> RouteHandler::normalRoutes;
RouteHandlerFunc RouteHandler::defualtHandler;

RouteHandler::RouteHandler() {}

RouteHandler::~RouteHandler() {}

void RouteHandler::addNormalRouteHandler(const struct Route &handler)
{
    normalRoutes.push_back(handler);
}

void RouteHandler::addPermissionRouteHandler(const struct Route &handler)
{
    permissionRoutes.push_back(handler);
}

void RouteHandler::setDefualtHandler(const RouteHandlerFunc handler)
{
    defualtHandler = handler;
}

const std::string &RouteHandler::getMimeType(const std::string &path,
                                             const LocationConfig &loc)
{
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
    {
        std::string extension = path.substr(dotPos + 1);
        std::map<std::string, std::string>::const_iterator it = loc.mimeTypes.find(extension);
        if (it != loc.mimeTypes.end())
        {
            return it->second;
        }
    }
    return loc.default_type; // Default MIME type
}

const libhttp::Response &RouteHandler::route(const libhttp::Request &request, const LocationConfig &location)
{
    std::vector<struct Route>::iterator it;

    for (it = permissionRoutes.begin();
         it != permissionRoutes.end();
         it++)
    {
        if ((*it).checker(request, location))
            return ((*it).handler(request, location));
    }

    for (it = normalRoutes.begin();
         it != normalRoutes.end();
         it++)
    {
        if ((*it).checker(request, location))
            return ((*it).handler(request, location));
    }

    if (defualtHandler)
        return defualtHandler(request, location);

    return generateDefaultErrorPage(500);
}

/**
 * generateErrorResponse() - Creates error response with custom error page
 *
 * Checks for custom error_page configuration first, falls back to default.
 *
 * PARAMETERS:
 * - status_code: HTTP status code (400, 403, 404, 500, etc.)
 * - config: Configuration that may contain custom error_page mappings
 *
 * RETURNS:
 * - Complete error response (custom page or default HTML)
 */
libhttp::Response &RouteHandler::generateErrorResponse(int status_code,
                                                       const LocationConfig &loc)
{
    static libhttp::StaticResponse response;

    response = libhttp::StaticResponse();
    // Check for custom error page
    std::map<int, CommonConfig::ErrorPage>::const_iterator it =
        loc.error_pages.find(status_code);

    if (it != loc.error_pages.end())
    {
        const CommonConfig::ErrorPage &error_page = it->second;
        std::string error_file_path = loc.root + error_page.uri;

        // Try to serve custom error page
        if (fileExists(error_file_path))
        {
            std::string content = readFile(error_file_path);

            response.setStatus(status_code);
            response.setHeader("Content-Type", "text/html");

            std::ostringstream ss;
            ss << content.size();
            response.setHeader("Content-Length", ss.str());

            response.setBody(content);
            return response;
        }
    }

    // Fall back to default error page
    return generateDefaultErrorPage(status_code);
}

/**
 * generateDefaultErrorPage() - Creates default HTML error page
 *
 * Used when no custom error_page is configured.
 */
libhttp::Response &RouteHandler::generateDefaultErrorPage(int status_code)
{
    static libhttp::StaticResponse response;

    std::string message = libhttp::getStatusMessage(status_code);

    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <title>" << status_code << " " << message << "</title>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>" << status_code << " " << message << "</h1>\n"
         << "  <hr>\n"
         << "  <p>webserv/1.0</p>\n"
         << "</body>\n"
         << "</html>\n";

    std::string body = html.str();

    response.setStatus(status_code);
    response.setHeader("Content-Type", "text/html");

    std::ostringstream ss;
    ss << body.size();
    response.setHeader("Content-Length", ss.str());

    response.setBody(body);

    return response;
}

/**
 * resolvePath() - Resolves URI to filesystem path using root or alias
 *
 * Handles:
 * - root directive: /images/cat.jpg -> <root>/images/cat.jpg
 * - alias directive: /photos/cat.jpg -> <alias>/cat.jpg (drops /photos)
 *
 * PARAMETERS:
 * - uri: Request URI (e.g., "/images/cat.jpg")
 * - location_path: Location block path (e.g., "/images")
 * - config: Configuration with root/alias settings
 *
 * RETURNS:
 * - Absolute filesystem path
 */
std::string RouteHandler::resolvePath(const std::string &uri,
                                      const std::string &location_path,
                                      const LocationConfig &loc)
{
    // Check for alias directive
    if (!loc.alias.empty())
    {
        // alias: strip location path from URI and prepend alias
        // Example: location /photos { alias /data/images; }
        // URI /photos/cat.jpg -> /data/images/cat.jpg

        std::string remaining_path = uri.substr(location_path.length());
        return loc.alias + remaining_path;
    }

    // Use root directive (default behavior)
    // Example: root /var/www;
    // URI /images/cat.jpg -> /var/www/images/cat.jpg
    return loc.root + uri;
}

/**
 * readFile() - Reads entire file into string
 *
 * RETURNS:
 * - File contents, or empty string if file cannot be read
 */
std::string RouteHandler::readFile(const std::string &path)
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

/**
 * fileExists() - Checks if file exists and is readable
 */
bool RouteHandler::fileExists(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

/**
 * isDirectory() - Checks if path is a directory
 */
bool RouteHandler::isDirectory(const std::string &path)
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
    {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

/**
 * findIndexFile() - Searches for index files in directory
 *
 * Checks each file in config.index list (e.g., index.html, index.htm).
 *
 * RETURNS:
 * - Path to first existing index file, or empty string if none found
 */
std::string RouteHandler::findIndexFile(const std::string &directory_path,
                                        const LocationConfig &loc)
{
    // Ensure directory path ends with /
    std::string dir = directory_path;
    if (!dir.empty() && dir[dir.length() - 1] != '/')
    {
        dir += "/";
    }

    // Check each index file in order
    for (size_t i = 0; i < loc.index.size(); ++i)
    {
        std::string index_path = dir + loc.index[i];

        if (fileExists(index_path) && !isDirectory(index_path))
        {
            return index_path;
        }
    }

    return ""; // No index file found
}
