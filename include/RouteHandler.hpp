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

typedef bool (*RouteChecherFunc)(const libhttp::Request &, const LocationConfig &);
typedef libhttp::Response &(*RouteHandlerFunc)(const libhttp::Request &, const LocationConfig &);

struct Route
{
    RouteChecherFunc checker;
    RouteHandlerFunc handler;
};

class RouteHandler
{
private:
    static std::vector<struct Route> permissionRoutes;
    static std::vector<struct Route> normalRoutes;
    static RouteHandlerFunc defualtHandler;
    // Disable constractor and copy
    RouteHandler();
    RouteHandler(const RouteHandler &other);
    RouteHandler &operator=(const RouteHandler &other);

public:
    virtual ~RouteHandler() = 0;

    static void addNormalRouteHandler(const struct Route &handler);
    static void addPermissionRouteHandler(const struct Route &handler);
    static void setDefualtHandler(const RouteHandlerFunc handler);

    static const libhttp::Response &route(const libhttp::Request &request, const LocationConfig &location);

    // ===== ERROR HANDLING =====

    static libhttp::Response &generateErrorResponse(int status_code,
                                                    const LocationConfig &loc);

    static libhttp::Response &generateDefaultErrorPage(int status_code);

    // ===== UTILITIES =====

    static std::string resolvePath(const std::string &uri,
                                   const std::string &location_path,
                                   const LocationConfig &loc);

    static std::string readFile(const std::string &path);

    static bool fileExists(const std::string &path);

    static bool isDirectory(const std::string &path);

    static std::string findIndexFile(const std::string &directory_path,
                                     const LocationConfig &loc);
};

#define DEFINE_NORMAL_ROUTE_HANDLER(checkHandler, routeHandler) \
    static struct _##checkHandler##routeHandler                 \
    {                                                           \
        _##checkHandler##routeHandler()                         \
        {                                                       \
            struct Route rh = {checkHandler, routeHandler};     \
            RouteHandler::addNormalRouteHandler(rh);            \
        }                                                       \
    } __##checkHandler##routeHandler

#define DEFINE_PERMISSION_ROUTE_HANDLER(checkHandler, routeHandler) \
    static struct _##checkHandler##routeHandler                     \
    {                                                               \
        _##checkHandler##routeHandler()                             \
        {                                                           \
            struct Route rh = {checkHandler, routeHandler};         \
            RouteHandler::addPermissionRouteHandler(rh);            \
        }                                                           \
    } __##checkHandler##routeHandler

#define SET_DEFUALT_HANDLER(routeHandler)                  \
    static struct _##routeHandler                          \
    {                                                      \
        _##routeHandler()                                  \
        {                                                  \
            RouteHandler::setDefualtHandler(routeHandler); \
        }                                                  \
    } __##routeHandler
