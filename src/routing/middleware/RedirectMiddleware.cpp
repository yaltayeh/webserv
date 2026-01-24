/**
 * RedirectMiddleware.cpp - Handle HTTP redirects (return directive)
 *
 * Middleware that handles the return directive for HTTP redirects.
 * Generates 301/302/etc redirect responses with Location header.
 */
#include "routing/RouteHandler.hpp"
#include "routing/RouteContext.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/ResponseBlender.hpp"

namespace routing
{
namespace middleware
{

/**
 * redirectHandler() - Middleware to handle redirects
 *
 * RETURNS:
 * - Response* (redirect) if return directive is set
 * - NULL to continue to next middleware
 */
libhttp::Response *redirectHandler(const libhttp::Request &request,
                                    const LocationConfig &location,
                                    RouteContext &context)
{
	(void)request; // Not used in this middleware
	(void)context; // Not used in this middleware

	// Check if return directive is configured
	if (!location.is_return)
		return NULL;

	// Generate redirect response
	libhttp::ResponseBlender blender = libhttp::ResponseBlender();
	blender.setStatus(location.return_code);
	blender.setHeader("Location", location.return_url);
	blender.setHeader("Content-Length", "0");
	
	libhttp::DefaultResponse *response = new libhttp::DefaultResponse(blender.toString());
	return response;
}

} // namespace middleware
} // namespace routing
