/**
 * AccessControlMiddleware.cpp - Check access control (deny directive)
 *
 * Middleware that checks if access is denied via the deny_all directive.
 * Returns 403 Forbidden if access is denied.
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "routing/RouteUtils.hpp"
#include "libhttp/include/Request.hpp"
#include "libutils/include/Logger.hpp"
#include "libhttp/include/Utils.hpp"

namespace routing
{
namespace middleware
{

/**
 * accessControlCheck() - Middleware to check access control
 *
 * RETURNS:
 * - Response* (403) if access is denied
 * - NULL to continue to next middleware
 */
libhttp::Response *accessControlCheck(const libhttp::Request &request,
                                       const LocationConfig &location,
                                       RouteContext &context)
{
	(void)request; // Not used in this middleware
	(void)context; // Not used in this middleware

	LOG_DEBUG("[AccessControlMiddleware] Handling request: " 
	          << libhttp::methodToString(request.getMethod()) << " " 
	          << request.getUri() << " | deny_all=" << location.deny_all);

	// Check if deny_all is enabled
	if (location.deny_all)
	{
		// Return 403 Forbidden
		return utils::generateErrorResponse(403, location);
	}

	// Access allowed, continue to next middleware
	return NULL;
}

} // namespace middleware
} // namespace routing
