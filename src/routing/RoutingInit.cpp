/**
 * RoutingInit.cpp - Routing system initialization implementation
 *
 * Registers all middleware with the RouteHandler singleton in the correct
 * priority order.
 */
#include "routing/RoutingInit.hpp"
#include "routing/RouteHandler.hpp"
#include <iostream>

namespace routing
{

void initializeRouting()
{
	// std::cout << "Initializing routing system..." << std::endl;

	// Build the RouteHandler with all middleware registered
	RouteHandler *handler = RouteHandler::builder()
		// Permission checks (Priority: 100 - highest)
		// NOTE: Body size is already checked in ClientSocket.cpp:105-112
		.use(middleware::accessControlCheck, 100)
		.use(middleware::methodCheck, 90)

		// Content handlers (Priority: 50)
		.use(middleware::redirectHandler, 50)
		
		// Upload handler (Priority: 30)
		.use(middleware::uploadHandler, 30)
		
		// Dynamic content (Priority: 25)
		.use(middleware::cgiHandler, 25)
		
		// Default/fallback handler (Priority: 0 - lowest, always handles)
		.use(middleware::staticFileHandler, 0)
		
		.build();

	// Set as singleton instance
	RouteHandler::setInstance(handler);

	// std::cout << "Routing system initialized successfully." << std::endl;
}

void cleanupRouting()
{
	// The RouteHandler singleton manages its own cleanup via setInstance(NULL)
	// or destructor, but we can explicitly clean it up here if needed
	// std::cout << "Cleaning up routing system..." << std::endl;
	RouteHandler::setInstance(NULL);
}

} // namespace routing
