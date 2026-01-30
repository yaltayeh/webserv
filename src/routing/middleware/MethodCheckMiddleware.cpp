/**
 * MethodCheckMiddleware.cpp - Validate HTTP method is allowed
 *
 * Middleware that checks if the HTTP method is in the allowed_methods set.
 * Returns 405 Method Not Allowed if method is not permitted.
 * HEAD is implicitly allowed wherever GET is allowed (per HTTP spec).
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "routing/RouteUtils.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/Utils.hpp"
#include "libutils/include/Logger.hpp"

namespace routing
{
namespace middleware
{

/**
 * methodCheck() - Middleware to validate HTTP method
 *
 * RETURNS:
 * - Response* (405) if method not allowed
 * - NULL to continue to next middleware
 */
libhttp::Response *methodCheck(const libhttp::Request &request,
                                const LocationConfig &location,
                                RouteContext &context)
{
	(void)context; // Not used in this middleware

	LOG_DEBUG("[MethodCheckMiddleware] Handling request: " 
	          << libhttp::methodToString(request.getMethod()) << " " 
	          << request.getUri());

	// If no restrictions configured, all methods are allowed
	if (location.allowed_methods.empty())
		return NULL;

	std::string method_str = libhttp::methodToString(request.getMethod());

	// HEAD is implicitly allowed wherever GET is allowed (HTTP spec)
	if (request.getMethod() == libhttp::HEAD)
	{
		if (location.allowed_methods.find("GET") != location.allowed_methods.end())
		{
			return NULL; // GET is allowed, so HEAD is also allowed
		}
	}

	// Check if method is in allowed set
	LOG_DEBUG("[MethodCheckMiddleware] Allowed methods: " << location.allowed_methods.size());
	if (location.allowed_methods.find(method_str) == location.allowed_methods.end())
	{
		// Method not allowed - return 405
		return utils::generateErrorResponse(405, location);
	}

	// Method allowed, continue to next middleware
	return NULL;
}

} // namespace middleware
} // namespace routing
