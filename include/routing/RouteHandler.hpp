/**
 * RouteHandler.hpp - Middleware-based HTTP Request Router
 *
 * WHAT THIS CLASS DOES:
 * Processes HTTP requests through a configurable chain of middleware functions.
 * Each middleware can either handle the request (return Response*) or pass to
 * the next middleware in the chain (return NULL).
 *
 * DESIGN PATTERN:
 * - Middleware Pipeline (Express.js/Connect-style)
 * - Builder Pattern for fluent configuration
 * - Singleton Pattern for global access
 * - Chain of Responsibility for request processing
 *
 * USAGE:
 *   // Initialization (once at startup)
 *   RouteHandler::setInstance(
 *       RouteHandler::builder()
 *           .use(bodyCheckMiddleware, 100)
 *           .use(accessControlMiddleware, 100)
 *           .use(methodCheckMiddleware, 90)
 *           .use(redirectMiddleware, 50)
 *           .use(cgiMiddleware, 25)
 *           .use(staticFileMiddleware, 0)
 *           .build()
 *   );
 *
 *   // Request handling
 *   Response* response = RouteHandler::getInstance().process(request, location);
 *   // ... send response ...
 *   delete response; // Caller owns the Response*
 *
 * MEMORY MANAGEMENT:
 * - process() returns Response* - caller MUST delete
 * - Middleware returns Response* to handle request
 * - Middleware returns NULL to pass to next handler
 * - If no middleware handles request, returns 500 error
 *
 * THREAD SAFETY:
 * - Stateless design (no per-request state in RouteHandler)
 * - Safe for concurrent use once initialized
 * - RouteContext provides request-specific storage
 */
#pragma once

#include "RouteContext.hpp"
#include "Config/LocationConfig.hpp"
#include "libhttp/include/Request.hpp"
#include "libhttp/include/Response.hpp"
#include <vector>

namespace routing
{

class RouteHandler
{
public:
	/**
	 * MiddlewareFunc - Middleware function signature
	 *
	 * PARAMETERS:
	 * - request: The HTTP request being processed
	 * - location: Matched location configuration
	 * - context: Shared context for middleware communication
	 *
	 * RETURNS:
	 * - Response* (non-NULL): Middleware handled the request, stop chain
	 * - NULL: Continue to next middleware in chain
	 *
	 * OWNERSHIP:
	 * - Returned Response* is owned by caller (must be deleted)
	 */
	typedef libhttp::Response *(*MiddlewareFunc)(const libhttp::Request &request,
	                                              const LocationConfig &location,
	                                              RouteContext &context);

	// Forward declaration of Builder
	class Builder;

	/**
	 * Destructor
	 */
	~RouteHandler();

	/**
	 * builder() - Creates a new Builder for fluent configuration
	 *
	 * RETURNS:
	 * - Builder instance for chaining .use() calls
	 *
	 * EXAMPLE:
	 *   RouteHandler* handler = RouteHandler::builder()
	 *       .use(middleware1, 100)
	 *       .use(middleware2, 50)
	 *       .build();
	 */
	static Builder builder();

	/**
	 * process() - Executes request through middleware chain
	 *
	 * PARAMETERS:
	 * - request: The HTTP request to process
	 * - location: The matched location configuration
	 *
	 * RETURNS:
	 * - Response* that MUST be deleted by caller
	 *
	 * BEHAVIOR:
	 * - Iterates through middleware in priority order (high to low)
	 * - Stops at first middleware that returns non-NULL Response*
	 * - Returns 500 error if no middleware handles the request
	 */
	libhttp::Response *process(const libhttp::Request &request,
	                           const LocationConfig &location) const;

	/**
	 * Singleton access methods
	 */
	static void setInstance(RouteHandler *handler);
	static RouteHandler &getInstance();

private:
	/**
	 * Middleware entry in the chain
	 */
	struct Middleware
	{
		MiddlewareFunc func;
		int priority; // Higher = executed first

		Middleware(MiddlewareFunc f, int p)
			: func(f), priority(p)
		{
		}

		// For sorting by priority (descending)
		bool operator<(const Middleware &other) const
		{
			return priority > other.priority; // Higher priority first
		}
	};

	std::vector<Middleware> _middlewares;

	// Singleton instance
	static RouteHandler *_instance;

	// Private constructor (use builder)
	RouteHandler();
};

/**
 * Builder - Fluent API for constructing RouteHandler
 *
 * USAGE:
 *   RouteHandler* handler = RouteHandler::builder()
 *       .use(middleware1)       // Default priority 0
 *       .use(middleware2, 100)  // High priority
 *       .build();
 */
class RouteHandler::Builder
{
public:
	Builder();

	/**
	 * use() - Adds middleware to the chain
	 *
	 * PARAMETERS:
	 * - middleware: Function pointer to middleware
	 * - priority: Execution priority (default 0)
	 *             Higher priority executes first
	 *
	 * RETURNS:
	 * - Reference to this Builder for chaining
	 *
	 * PRIORITY GUIDELINES:
	 * - 100: Permission checks (body size, access control, method validation)
	 * - 50:  Content handlers (redirects, special routes)
	 * - 25:  Dynamic content (CGI, FastCGI)
	 * - 0:   Default handlers (static files, fallback)
	 */
	Builder &use(MiddlewareFunc middleware, int priority = 0);

	/**
	 * build() - Constructs the RouteHandler
	 *
	 * RETURNS:
	 * - Heap-allocated RouteHandler* (caller owns)
	 *
	 * NOTES:
	 * - Sorts middleware by priority before building
	 * - Builder can be discarded after build()
	 */
	RouteHandler *build();

private:
	std::vector<Middleware> _middlewares;
};

} // namespace routing
