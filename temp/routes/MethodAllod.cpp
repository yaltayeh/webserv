#include "RouteHandler.hpp"
#include "libhttp/include/Utils.hpp"
#include <iostream>

// Validate HTTP method
// if (!isMethodAllowed(request.getMethod(), location))
// {
//     return generateErrorResponse(405, location);
// }

/**
 * isMethodAllowed() - Checks if HTTP method is allowed for this location
 *
 * HEAD is automatically allowed wherever GET is allowed, per HTTP specification.
 *
 * RETURNS:
 * - true if method is allowed (or no restrictions configured)
 * - false if method is explicitly denied
 */
static bool isMethodNotAllowed(const libhttp::Request &request,
                            const LocationConfig &loc)
{
    // If no restrictions configured, all methods are allowed
    if (loc.allowed_methods.empty())
    {
        return false;
    }

    std::string method_str = libhttp::methodToString(request.getMethod());

    // HEAD is implicitly allowed wherever GET is allowed
    if (request.getMethod() == libhttp::HEAD)
    {
        if (loc.allowed_methods.find("GET") != loc.allowed_methods.end())
        {
            return false; // GET is allowed, so HEAD is also allowed
        }
    }

    return loc.allowed_methods.find(method_str) == loc.allowed_methods.end();
}

static libhttp::Response &generateResponse(const libhttp::Request &request,
                                           const LocationConfig &loc)
{
    (void)request;
    return RouteHandler::generateErrorResponse(405, loc);
}

DEFINE_PERMISSION_ROUTE_HANDLER(isMethodNotAllowed, generateResponse);
