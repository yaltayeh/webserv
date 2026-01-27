#include <iostream>
#include <csignal>
#include "Config/GlobalConfig.hpp"
#include "libdir/include/Parser.hpp"
#include "EventLoop.hpp"
#include "Parser.hpp"
#include "routing/RoutingInit.hpp"
#include "libutils/include/Logger.hpp"

// Global pointer to event loop for signal handler
static EventLoop* g_eventLoop = NULL;

// Signal handler for graceful shutdown
void signalHandler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		LOG_INFO("Shutdown signal received, stopping server...");
		if (g_eventLoop)
		{
			g_eventLoop->stop();
		}
	}
}

int main(int argc, const char *argv[], const char *envp[])
{
	if (argc != 2)
	{
		LOG_INFO("Usage: " << argv[0] << " <config-file>");
		return 1;
	}
	// TODO: You need at least the GET, POST, and DELETE methods.
	// TODO: Your server must have default error pages if none are provided.
	// TODO: Your program must use a configuration file, provided as an argument on the command line, or available in a default path.
	GlobalConfig config = GlobalConfig();
	try
	{
		Directive::Parser::loadConfigFile(argv[1], config, envp);

		// Initialize routing system before starting event loop
		routing::initializeRouting();

		EventLoop loop(config);
		
		// Set up signal handlers for graceful shutdown
		g_eventLoop = &loop;
		std::signal(SIGINT, signalHandler);
		std::signal(SIGTERM, signalHandler);
		
		LOG_INFO("Server started. Press Ctrl+C to stop.");
		loop.run();

		// Cleanup routing when done
		routing::cleanupRouting();
	}
	catch (const std::exception &e)
	{
		LOG_ERROR(e.what());
		routing::cleanupRouting(); // Cleanup even on error
		return 2;
	}
	return 0;
}


// TODO: However, you are allowed to use fcntl() only with the following flags:
// 	F_SETFL, O_NONBLOCK and, FD_CLOEXEC.
// 	Any other flag is forbidden.

/*
	yacob
	[x] worker_connections
	[x] idle_timeout
	[x] the queue system.
	[x] the level trigger/trigger mode.
	[x] the handleDistroy for IOEventHandler.

	ahmed
	[x] getChunk implementation
	[x] checkChunk implementation
	[x] HEAD method implementation
	[x] fix luka extract test case
	[ ] signal to stop the server gracefully 
*/