#include "RouteHandler.hpp"
#include <iostream>
#include <cstdlib>

/**
 * exceedsMaxBodySize() - Checks if request body exceeds limit
 *
 * RETURNS:
 * - true if Content-Length exceeds client_max_body_size
 * - false otherwise
 */
static bool exceedsMaxBodySize(const libhttp::Request &request,
						const LocationConfig &loc)
{
	std::string content_length_str = request.getHeader("Content-Length");

	if (content_length_str.empty())
	{
		return false; // No Content-Length header, cannot check
	}

	// Parse Content-Length
	size_t content_length = std::atol(content_length_str.c_str());

	return content_length > loc.client_max_body_size;
}

static libhttp::Response &generateResponse(const libhttp::Request &request,
										   const LocationConfig &loc)
{
	(void)request;
	return RouteHandler::generateErrorResponse(413, loc);
}

DEFINE_PERMISSION_ROUTE_HANDLER(exceedsMaxBodySize, generateResponse);
