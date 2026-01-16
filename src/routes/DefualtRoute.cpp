#include <iostream>
#include <sstream>
#include "libhttp/include/Utils.hpp"
#include "RouteHandler.hpp"
#include <dirent.h>
#include <sys/stat.h>

/**
 * handleAutoindex() - Generates directory listing
 *
 * Lists files and subdirectories in HTML format when:
 * - No index file found
 * - autoindex is enabled
 */
libhttp::Response &handleAutoindex(const std::string &directory_path,
                                                const std::string &uri)
{
    static libhttp::StaticResponse response;

    // Open directory
    DIR *dir = opendir(directory_path.c_str());
    if (dir == NULL)
    {
        response.setStatus(500);
        response.setBody("Failed to open directory");
        return response;
    }

    // Build HTML directory listing
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <title>Index of " << uri << "</title>\n"
         << "  <style>\n"
         << "    body { font-family: monospace; }\n"
         << "    table { border-collapse: collapse; }\n"
         << "    th, td { text-align: left; padding: 8px; }\n"
         << "    tr:hover { background-color: #f5f5f5; }\n"
         << "  </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>Index of " << uri << "</h1>\n"
         << "  <table>\n"
         << "    <tr><th>Name</th><th>Size</th></tr>\n";

    // Add parent directory link if not root
    if (uri != "/")
    {
        html << "    <tr><td><a href=\"../\">../</a></td><td>-</td></tr>\n";
    }

    // Read directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        // Skip . and ..
        if (name == "." || name == "..")
        {
            continue;
        }

        // Get file info
        std::string full_path = directory_path + "/" + name;
        struct stat st;

        if (stat(full_path.c_str(), &st) == 0)
        {
            std::string display_name = name;
            std::string size_str;

            if (S_ISDIR(st.st_mode))
            {
                display_name += "/";
                size_str = "-";
            }
            else
            {
                std::ostringstream ss;
                ss << st.st_size;
                size_str = ss.str();
            }

            // Ensure URI ends with / for proper relative links
            std::string base_uri = uri;
            if (!base_uri.empty() && base_uri[base_uri.length() - 1] != '/')
            {
                base_uri += "/";
            }

            html << "    <tr>"
                 << "<td><a href=\"" << base_uri << name << "\">"
                 << display_name << "</a></td>"
                 << "<td>" << size_str << "</td>"
                 << "</tr>\n";
        }
    }

    closedir(dir);

    html << "  </table>\n"
         << "</body>\n"
         << "</html>\n";

    std::string body = html.str();

    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");

    std::ostringstream ss;
    ss << body.size();
    response.setHeader("Content-Length", ss.str());

    response.setBody(body);

    return response;
}

/**
 * handleStaticFile() - Serves a file from the filesystem
 *
 * Handles:
 * - Reading file from disk
 * - Setting Content-Type based on file extension
 * - Setting Content-Length
 * - Directory index (index.html, etc.)
 * - Autoindex (directory listing) if enabled
 * - 404 if file not found
 * - 403 if not readable
 */
libhttp::Response &handleStaticFile(const libhttp::Request &request,
                                    const LocationConfig &loc)
{
    std::cout << "handleStaticFile" << std::endl;
    static libhttp::StaticResponse response;
    const std::string &uri = request.getUri();

    // Get location path (empty string if server-level config)
    std::string location_path = loc.path;

    // Resolve URI to filesystem path
    std::string file_path = RouteHandler::resolvePath(uri, location_path, loc);

    std::cout << "Resolving URI '" << uri << "' to path: " << file_path << std::endl;

    // Check if path exists
    if (!RouteHandler::fileExists(file_path))
    {
        return RouteHandler::generateErrorResponse(404, loc);
    }

    // Handle directory
    if (RouteHandler::isDirectory(file_path))
    {
        // Look for index file
        std::string index_file = RouteHandler::findIndexFile(file_path, loc);

        if (!index_file.empty())
        {
            file_path = index_file;
        }
        else if (loc.autoindex)
        {
            // Generate directory listing
            std::cout << "Autoindex enabled, generating directory listing" << std::endl;
            return handleAutoindex(file_path, uri);
        }
        else
        {
            // No index file and autoindex disabled
            std::cout << "Autoindex disabled (autoindex=" << loc.autoindex << ")" << std::endl;
            return RouteHandler::generateErrorResponse(403, loc);
        }
    }

    // Read file contents
    std::string content = RouteHandler::readFile(file_path);
    if (content.empty() && !RouteHandler::fileExists(file_path))
    {
        return RouteHandler::generateErrorResponse(404, loc);
    }

    // Build response
    response.setStatus(200);
    response.setHeader("Content-Type", libhttp::getMimeType(file_path));

    std::ostringstream ss;
    ss << content.size();
    response.setHeader("Content-Length", ss.str());

    response.setBody(content);

    return response;
}

SET_DEFUALT_HANDLER(handleStaticFile);
