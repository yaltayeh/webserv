#include "routing/RouteHandler.hpp"
#include "libutils/include/Logger.hpp"
#include "libhttp/include/DefaultResponse.hpp"
#include "libhttp/include/ResponseBlender.hpp"
#include "libhttp/include/Utils.hpp"
#include <algorithm>
#include <iostream>

namespace routing
{

// Initialize static singleton instance
RouteHandler *RouteHandler::_instance = NULL;

// ==================== RouteHandler Implementation ====================
RouteHandler::RouteHandler()
	: _middlewares() { }
RouteHandler::~RouteHandler() { }






libhttp::Response *RouteHandler::process(const libhttp::Request &request,
                                          const LocationConfig &location) const
{
	// Create request context
	RouteContext context;

	// Execute middleware chain in priority order
	for (size_t i = 0; i < _middlewares.size(); ++i)
	{
		const Middleware &middleware = _middlewares[i];

		// Call middleware
		libhttp::Response *response = middleware.func(request, location, context);

		// If middleware handled the request, return its response
		if (response != NULL)
		{
			return response;
		}

		// If context says stop, but no response returned, that's an error
		if (!context.shouldContinue)
		{
			LOG_ERROR("Middleware stopped chain but returned NULL response");
			break;
		}
	}

	// No middleware handled the request - return 500 Internal Server Error
	LOG_ERROR("No middleware handled request for URI: " << request.getUri());

	libhttp::ResponseBlender *blender = new libhttp::ResponseBlender();
	blender->setStatus(500);
	blender->setHeader("Content-Type", "text/html");

	std::string body = "<!DOCTYPE html>\n"
	                   "<html>\n"
	                   "<head><title>500 Internal Server Error</title></head>\n"
	                   "<body>\n"
	                   "<h1>500 Internal Server Error</h1>\n"
	                   "<p>No handler available for this request.</p>\n"
	                   "<hr>\n"
	                   "<p>webserv/1.0</p>\n"
	                   "</body>\n"
	                   "</html>\n";

	blender->setBody(body);

	libhttp::DefaultResponse *errorResponse = new libhttp::DefaultResponse(blender->toString());
	return errorResponse;
}

RouteHandler::Builder RouteHandler::builder()
{
	return Builder();
}

void RouteHandler::setInstance(RouteHandler *handler)
{
	if (_instance != NULL)	{ delete _instance; }
	_instance = handler;
}

RouteHandler &RouteHandler::getInstance()
{
	if (_instance == NULL)
	{
		LOG_CRITICAL("RouteHandler::getInstance() called before setInstance()");
		// Return a default instance to avoid crash, but this is a programming error
		static RouteHandler defaultInstance;
		return defaultInstance;
	}
	return *_instance;
}





// ==================== Builder Implementation ====================
RouteHandler::Builder::Builder()
	: _middlewares() { }

RouteHandler::Builder &RouteHandler::Builder::use(MiddlewareFunc middleware, int priority)
{
	_middlewares.push_back(Middleware(middleware, priority));
	return *this;
}

RouteHandler *RouteHandler::Builder::build()
{
	RouteHandler *handler = new RouteHandler();

	// Copy middlewares to handler
	handler->_middlewares = _middlewares;

	// Sort by priority (descending - highest priority first)
	std::sort(handler->_middlewares.begin(), handler->_middlewares.end());

	return handler;
}

} // namespace routing
