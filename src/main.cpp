#include <iostream>
#include "Config/GlobalConfig.hpp"
#include "libdir/include/Parser.hpp"
#include "EventLoop.hpp"
#include "Parser.hpp"
#include "routing/RoutingInit.hpp"

int main(int argc, const char *argv[], const char *envp[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <config-file>" << std::endl;
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
		loop.run();

		// Cleanup routing when done
		routing::cleanupRouting();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		routing::cleanupRouting(); // Cleanup even on error
		return 2;
	}
	return 0;
}


// TODO: However, you are allowed to use fcntl() only with the following flags:
// 	F_SETFL, O_NONBLOCK and, FD_CLOEXEC.
// 	Any other flag is forbidden.

// TODO: You must provide configuration files and default files to test and demonstrate that
// 	every feature works during the evaluation.