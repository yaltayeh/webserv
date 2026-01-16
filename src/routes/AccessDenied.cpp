#include "RouteHandler.hpp"
#include "libhttp/include/Response.hpp"

/**
 * isAccessDenied() - Checks if access is denied (deny directive)
 *
 * RETURNS:
 * - true if deny_all is enabled
 * - false otherwise
 */
#include <iostream>
bool checkAccessDenied(const libhttp::Request &r, const LocationConfig &loc)
{
	std::cout << "checkAccessDenied" << std::endl;
	(void)r;
	return loc.deny_all;
}

static libhttp::Response &generateResponse(const libhttp::Request &request, const LocationConfig &loc)
{
	(void)request;
	return RouteHandler::generateErrorResponse(403, loc);
}


DEFINE_PERMISSION_ROUTE_HANDLER(checkAccessDenied, generateResponse);
