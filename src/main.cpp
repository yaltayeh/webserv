#include "../include/EventLoop.hpp"
#include "../include/Server.hpp"
#include "../include/config.hpp"
#include <iostream>

#include "libdir/include/Parser.hpp"
#include "libdir/include/Config.hpp"
#include "HttpDirective.hpp"
#include "ClientBodyTempPathDirective.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"

/**
 * ============================================================================
 * main() - Program entry point
 * ============================================================================
 * 
 * EXECUTION FLOW:
 * 0. Parse configuration file into directive tree
 * 1. Create EventLoop instance
 * 2. Create Server instances with different configurations (ports)
 * 3. Initialize each Server (creates listening sockets)
 * 4. Add each Server to EventLoop
 * 5. Start EventLoop (never returns - runs until killed)
 * 
 */
int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
		return 1;
	}
	
    /**
     * ========================================================================
     * STEP 0: Parse configuration file
     * ========================================================================
     */
	try {
		Config::Parser parser = Config::Parser::openConfigFile(argv[1]);

		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			if (!Config::IDirective::parseDirective(Config::global, directive)) { 
				std::cerr << "Failed to parse directive" << std::endl;
				return 1; 
			}
		}
		
		std::cout << "Configuration parsed successfully" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error parsing configuration: " << e.what() << std::endl;
		return 1;
	}
    
    /**
     * ========================================================================
     * STEP 1: Create EventLoop
     * ========================================================================
     * 
     * The EventLoop will own all Server instances and coordinate their
     * I/O operations through a single poll() call.
     */
    EventLoop eventLoop;
    
    /**
     * ------------------------------------------------------------------------
     * STEP 2: Create Servers from Configuration
     * ------------------------------------------------------------------------
     */

    const Config::ListDirectives<Config::ServerDirective> servers = Config::global
        ->getOne<Config::HttpDirective>()
        ->getMany<Config::ServerDirective>();

    std::cout << "Found " << servers.size() << " server(s) in configuration" << std::endl;

    for (size_t i = 0; i < servers.size(); ++i) {
        Server* server = new Server(
            servers[i]->toServerConfig()
        );
        
        if (server->initialize() == -1) {
            std::cerr << "Failed to initialize server on port "
                      << server->getConfig().port << std::endl;
            delete server;
            return 1;
        }
        
        eventLoop.addServer(server);
    }

    /**
     * ========================================================================
     * STEP 3: Verify we have at least one server
     * ========================================================================
     */
    if (eventLoop.getServerCount() == 0) {
        std::cerr << "Error: No servers initialized successfully" << std::endl;
        return 1;
    }
    
    /**
     * ========================================================================
     * STEP 4: Start the event loop
     * ========================================================================
     */
    eventLoop.run();
    
    return 0;
}