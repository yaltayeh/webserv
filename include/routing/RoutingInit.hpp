/**
 * RoutingInit.hpp - Routing system initialization
 *
 * This module provides the initialization function that sets up the
 * RouteHandler singleton with all registered middleware.
 *
 * Call initializeRouting() once at application startup before handling
 * any requests.
 */
#pragma once

#include "routing/RouteHandler.hpp"

namespace routing
{

// Forward declare all middleware functions
namespace middleware
{
	// Permission check middlewares (Priority: 100)
	// NOTE: Body size is checked in ClientSocket.cpp:105-112 (parser level)
	
	libhttp::Response *accessControlCheck(const libhttp::Request &request,
	                                       const LocationConfig &location,
	                                       RouteContext &context);

	libhttp::Response *methodCheck(const libhttp::Request &request,
	                                const LocationConfig &location,
	                                RouteContext &context);

	// Content handler middlewares (Priority: 50)
	libhttp::Response *redirectHandler(const libhttp::Request &request,
	                                    const LocationConfig &location,
	                                    RouteContext &context);

	// Dynamic content middlewares (Priority: 25)
	libhttp::Response *cgiHandler(const libhttp::Request &request,
	                               const LocationConfig &location,
	                               RouteContext &context);

	// Default handler middlewares (Priority: 0)
	libhttp::Response *staticFileHandler(const libhttp::Request &request,
	                                      const LocationConfig &location,
	                                      RouteContext &context);
}

/**
 * initializeRouting() - Initialize the routing system
 *
 * MUST be called once at application startup before handling any requests.
 * Sets up the RouteHandler singleton with all middleware in the correct order.
 *
 * MIDDLEWARE PRIORITY ORDER:
 * - 100: Permission checks (access control, method validation)
 * - 50:  Content handlers (redirects, special routes)
 * - 25:  Dynamic content (CGI, FastCGI)
 * - 0:   Default handlers (static files - always handles request)
 *
 * NOTE: Body size validation is done at the parser level (ClientSocket.cpp:105)
 */
void initializeRouting();

/**
 * cleanupRouting() - Clean up routing resources
 *
 * Should be called at application shutdown.
 * Frees the RouteHandler singleton.
 */
void cleanupRouting();

} // namespace routing
