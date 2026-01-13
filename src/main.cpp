#include "../include/EventLoop.hpp"
#include "../include/Server.hpp"
#include "../include/Config/ServerConfig.hpp"
#include <iostream>

#include "libdir/include/Parser.hpp"
#include "libdir/include/Config.hpp"
#include "Config/GlobalConfig.hpp"
#include "Config/ServerConfig.hpp"

/**
 * printUsage() - Prints program usage information
 */
void printUsage(const char *progName) {
    std::cout << "Usage: " << progName << " <config-file>" << std::endl;
}

/**
 * initializeServers() - Initialize and add servers from config to event loop
 */
int initializeServers(const GlobalConfig &config, EventLoop &eventLoop) {
    for (size_t i = 0; i < config.https.size(); i++) {
        std::vector<ServerConfig *> servers = config.https[i]->servers;
        for (size_t j = 0; j < servers.size(); j++) {
            std::cout << "listenAddresses: " 
                << servers[j]->listenAddresses.size()
                << std::endl;
            Server *server = new Server(*servers[j]);

            if (server->initialize() == -1) {
                std::cerr << "Failed to initialize server" << std::endl;
                delete server;
                return -1;
            }

            eventLoop.addServer(server);
        }
    }


    if (eventLoop.getServerCount() == 0) {
        std::cerr << "Error: No servers initialized successfully" << std::endl;
        return 1;
    }

    return 0;
}

/**
 * main() - Program entry point
 *
 * EXECUTION FLOW:
    * 0. Validate command line arguments
    * 1. Parse configuration file into GlobalConfig
    * 2. Create EventLoop instance
    * 3. Initialize Server instances from configuration and add to EventLoop
    * 4. Start EventLoop to handle connections
 *
 */
int main(int argc, char *argv[]) {
    // STEP 0: Validate command line arguments
    if (argc != 2) { printUsage(argv[0]); return 1; }


    // STEP 1: Parse configuration file
    GlobalConfig config = GlobalConfig();
    try {
        Directive::Parser parser = Directive::Parser::openConfigFile(argv[1]);
        Directive::Parser::loadConfigFile(argv[1], config);
    }
    catch (const std::exception &e) {
        std::cerr << "Error parsing configuration: " << e.what() << std::endl;
        return 1;
    }


    // STEP 2: Create EventLoop:
    //     The EventLoop will own all Server instances and coordinate their
    //     I/O operations through a single poll() call.
    EventLoop eventLoop;


    // STEP 3: Initialize servers from configuration
    if (initializeServers(config, eventLoop) == -1)
        return 1;


    // STEP 4: Start the event loop
    eventLoop.run();


    return 0;
}