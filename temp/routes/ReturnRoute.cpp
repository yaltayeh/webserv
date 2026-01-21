#include "RouteHandler.hpp"
#include <iostream>

// STEP 6: Handle redirects (return directive)
// if (location.is_return)
// {
//     return handleRedirect(location.return_code,
//                           location.return_url);
// }

static bool isRedirection(const libhttp::Request &request,
						  const LocationConfig &loc)
{
	(void)request;
	return loc.is_return;
}

/**
 * handleRedirect() - Generates HTTP redirect response
 *
 * Handles return directive:
 * - return 301 /new-location;
 * - return 302 https://example.com;
 */
static libhttp::Response &handleRedirect(const libhttp::Request &request,
										 const LocationConfig &loc)
{
	(void)request;
	static libhttp::DefaultResponse response;
    response = libhttp::DefaultResponse();

	response.setStatus(loc.return_code);
	response.setHeader("Location", loc.return_url);
	response.setHeader("Content-Length", "0");

	return response;
}

DEFINE_NORMAL_ROUTE_HANDLER(isRedirection, handleRedirect);
