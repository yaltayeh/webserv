/**
 * RouteHandler.hpp - HTTP Request Routing and Response Generation
 *
 * WHAT THIS CLASS DOES:
 * Handles the complete request-response cycle by routing HTTP requests to the
 * appropriate handler (static file, CGI script, redirect, error page) based on
 * the server configuration and request URI.
 *
 * RESPONSIBILITIES:
 * - Match request URI against server location blocks
 * - Serve static files from the filesystem
 * - Execute CGI scripts and return their output
 * - Handle HTTP redirects (return directive)
 * - Generate error responses (403, 404, 405, 413, 500, etc.)
 * - Validate allowed methods per location
 * - Check access control (deny directive)
 *
 * DESIGN:
 * - Stateless: Each route() call is independent
 * - Uses ServerConfig for routing rules
 * - Delegates to libhttp::Response for response construction
 * - Delegates to CGI class for script execution
 *
 */
#pragma once

#include "Config/ServerConfig.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/Response.hpp"
#include "libhttp/include/StaticResponse.hpp"
#include "libhttp/include/StreamResponse.hpp"
#include <string>

namespace RouteRequest
{
    const libhttp::Response &route(const libhttp::Request &request, const LocationConfig &location);

    // ===== ERROR HANDLING =====

    libhttp::Response &generateErrorResponse(int status_code,
                                                    const LocationConfig &loc);

    libhttp::Response &generateDefaultErrorPage(int status_code);

    // ===== UTILITIES =====

    std::string resolvePath(const std::string &uri,
                                   const std::string &location_path,
                                   const LocationConfig &loc);

     std::string readFile(const std::string &path);

     bool fileExists(const std::string &path);

     bool isDirectory(const std::string &path);

     std::string findIndexFile(const std::string &directory_path,
                                     const LocationConfig &loc);

     const std::string &getMimeType(const std::string &path,
                                          const LocationConfig &loc);
};