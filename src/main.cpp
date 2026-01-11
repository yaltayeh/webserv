#include "../include/EventLoop.hpp"
#include "../include/Server.hpp"
#include "../include/Config/ServerConfig.hpp"
#include <iostream>

#include "libdir/include/Parser.hpp"
#include "libdir/include/Config.hpp"
#include "HttpDirective.hpp"
#include "ClientBodyTempPathDirective.hpp"
#include "ServerDirective.hpp"
#include "ListenDirective.hpp"


void printUsage(const char* progName)
{
    std::cout << "Usage: " << progName << " <config-file>" << std::endl;
}

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
    // Validate command line arguments
	if (argc != 2) { printUsage(argv[0]); return 1; }
	
    // STEP 0: Parse configuration file
	try {
		Directive::Parser parser = Directive::Parser::openConfigFile(argv[1]);

		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			if (!Directive::IDirective::parseDirective(Directive::global, directive)) { 
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
    
    // STEP 2: Create Servers from Configuration
    const Directive::ListDirectives<Directive::ServerDirective> servers = Directive::global
        ->getOne<Directive::HttpDirective>()
        ->getMany<Directive::ServerDirective>();

    std::cout << "Found " << servers.size() << " server(s) in configuration" << std::endl;

    for (size_t i = 0; i < servers.size(); ++i) {
        Server* server = new Server(
            ServerConfig::fromDirective(servers[i])
        );
        
        if (server->initialize() == -1) {
            std::cerr << "Failed to initialize server on port "
                      << server->getConfig().port << std::endl;
            delete server;
            return 1;
        }
        
        eventLoop.addServer(server);
    }

    // STEP 3: Verify we have at least one server
    if (eventLoop.getServerCount() == 0) {
        std::cerr << "Error: No servers initialized successfully" << std::endl;
        return 1;
    }
    
    // STEP 4: Start the event loop
    eventLoop.run();
    
    return 0;
}