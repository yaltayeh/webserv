/**
 * ============================================================================
 * main.cpp - HTTP Server Entry Point
 * ============================================================================
 * 
 * This is the main entry point for the webserv HTTP server. It demonstrates
 * the multi-server architecture where multiple server instances (different
 * host:port pairs) run simultaneously in a single event loop with one unified
 * poll() call.
 * 
 * ARCHITECTURE OVERVIEW:
 * - EventLoop: Owns all servers and manages the unified poll() call
 * - Server: Represents one listening endpoint (host:port) with configuration
 * - ConnectionManager: Manages all client connections across all servers
 * - Connection: Represents one client socket with parser and write buffer
 * 
 * EXECUTION FLOW:
 * 1. Create EventLoop instance
 * 2. Create Server instances with different configurations (ports)
 * 3. Initialize each Server (creates listening sockets)
 * 4. Add each Server to EventLoop
 * 5. Start EventLoop (never returns - runs until killed)
 * 
 * ============================================================================
 */

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
 * LEARNING ROADMAP - Implementation Progress
 * ============================================================================
 * 
 * PHASE 1: Basic Socket Server ✓ COMPLETE
 *   - Created listening socket infrastructure
 *   - Implemented in Server class with socket(), bind(), listen()
 * 
 * PHASE 2: Non-Blocking I/O ✓ COMPLETE
 *   - All sockets set to O_NONBLOCK with fcntl()
 *   - Server and ConnectionManager enforce non-blocking mode
 * 
 * PHASE 3: I/O Multiplexing with poll() ✓ COMPLETE
 *   - Single poll() call monitors ALL sockets (listening + clients)
 *   - EventLoop coordinates dispatch to Server and ConnectionManager
 *   - Multiple servers supported with unified poll() array
 * 
 * PHASE 4: Multi-Server Architecture ✓ COMPLETE
 *   - Server class encapsulates per-server state and configuration
 *   - EventLoop manages multiple Server instances
 *   - Connections associated with originating Server via owner_server pointer
 * 
 * PHASE 5: HTTP Protocol Integration ⚙ IN PROGRESS
 *   - libhttp::Parser integrated for incremental request parsing
 *   - Basic response generation works
 *   - TODO: Implement request routing based on ServerConfig
 * 
 * PHASE 6: Configuration File Parser - TODO
 *   - Parse NGINX-like config files
 *   - Populate ServerConfig and RouteConfig structures
 *   - Support multiple server blocks, routes, error pages
 * 
 * PHASE 7: Request Routing and File Serving - TODO
 *   - Match request URI against routes in ServerConfig
 *   - Serve static files from configured root directories
 *   - Handle directory listing and index files
 *   - Generate proper error responses (404, 403, 500)
 * 
 * PHASE 8: CGI Execution - TODO
 *   - Fork and execute CGI scripts (php-cgi, python, etc.)
 *   - Pass request data via environment variables and stdin
 *   - Capture CGI output and send to client
 *   - Handle chunked request un-chunking for CGI
 * 
 * PHASE 9: File Uploads - TODO
 *   - Parse multipart/form-data POST requests
 *   - Write uploaded files to configured directories
 *   - Enforce client_max_body_size limits
 * 
 * ============================================================================
 */

/**
 * ============================================================================
 * createDefaultServerConfig() - Creates a basic server configuration
 * ============================================================================
 * 
 * WHAT THIS DOES:
 * Creates a temporary ServerConfig for testing/development. In production,
 * this would be replaced by configuration file parsing.
 * 
 * PARAMETERS:
 * - host: IP address to bind to (e.g., "0.0.0.0", "127.0.0.1")
 * - port: Port number to listen on (e.g., 8080, 8081)
 * 
 * RETURNS:
 * ServerConfig object with basic settings
 */
static ServerConfig createDefaultServerConfig(const std::string& host, int port)
{
    ServerConfig config;
    
    /**
     * Basic server settings
     */
    config.host = host;
    config.port = port;
    config.server_name = "localhost";
    
    /**
     * Error pages (using default pages for now)
     * 
     * TODO: Load custom error pages from disk
     */
    config.error_pages[404] = "/errors/404.html";
    config.error_pages[500] = "/errors/500.html";
    
    /**
     * Body size limit: 10 MB
     */
    config.client_max_body_size = 10485760;
    
    /**
     * Create a default catch-all route
     * 
     * This route matches all URIs (path = "/") and serves as the
     * fallback for requests that don't match more specific routes.
     */
    RouteConfig default_route;
    default_route.path = "/";
    default_route.root = "./www";  /** Serve files from ./www directory */
    default_route.directory_listing = false;
    default_route.index_files.push_back("index.html");
    
    /**
     * Add the default route to server config
     */
    config.routes.push_back(default_route);
    
    return config;
}

/**
 * ============================================================================
 * main() - Program entry point
 * ============================================================================
 * 
 * WHAT THIS DOES:
 * 1. Creates an EventLoop instance
 * 2. Creates multiple Server instances (different ports for testing)
 * 3. Initializes each Server (creates listening sockets)
 * 4. Registers servers with EventLoop
 * 5. Starts the event loop (runs forever)
 * 
 * RETURNS:
 * - 0 on success (never reached - event loop is infinite)
 * - 1 on initialization failure
 */
int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
		return 1;
	}
	
	Config::Parser parser = Config::Parser::openConfigFile(argv[1]);

	while (parser.getNextDirectives()) {
		std::string directive = parser.copyDirective();
		if (!Config::IDirective::parseDirective(Config::global, directive)) { return 1; }
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
     * ========================================================================
     * STEP 2: Create and configure Server instances
     * ========================================================================
     * 
     * For demonstration, we create multiple servers on different ports.
     * In production, these would be loaded from a configuration file.
     * 
     * MULTI-SERVER EXAMPLE:
     * - Server 1: Port 8080 (main server)
     * - Server 2: Port 8081 (admin interface)
     * - Server 3: Port 8082 (API endpoint)
     * 
     * Each server can have different routes, error pages, and settings.
     */
    
    /**
     * ------------------------------------------------------------------------
     * Server 1: Port 8080
     * ------------------------------------------------------------------------
     */

    const Config::ListDirectives<Config::ServerDirective> servers = Config::global
        ->getOne<Config::HttpDirective>()
        ->getMany<Config::ServerDirective>();

    for (size_t i = 0; i < servers.size(); ++i) {
        ServerConfig config = servers[i]->toServerConfig();
        Server* server = new Server(config);
        
        if (server->initialize() == -1) {
            std::cerr << "Failed to initialize server on port "
                      << config.port << std::endl;
            delete server;
            return 1;
        }
        
        eventLoop.addServer(server);
    }

    /**
     * ------------------------------------------------------------------------
     * Server 2: Port 8081 (optional - comment out if you want only one server)
     * ------------------------------------------------------------------------
     * 
     * This demonstrates the multi-server capability. Uncomment to run
     * multiple servers simultaneously.
     */
    /*
    std::cout << "Creating server on port 8081..." << std::endl;
    
    ServerConfig config2 = createDefaultServerConfig("0.0.0.0", 8081);
    Server* server2 = new Server(config2);
    
    if (server2->initialize() == -1) {
        std::cerr << "Failed to initialize server on port 8081" << std::endl;
        delete server2;
        // Continue with server1 only
    } else {
        eventLoop.addServer(server2);
    }
    */
    
    /**
     * ------------------------------------------------------------------------
     * Server 3: Port 8082 (optional - another example)
     * ------------------------------------------------------------------------
     */
    /*
    std::cout << "Creating server on port 8082..." << std::endl;
    
    ServerConfig config3 = createDefaultServerConfig("0.0.0.0", 8082);
    Server* server3 = new Server(config3);
    
    if (server3->initialize() == -1) {
        std::cerr << "Failed to initialize server on port 8082" << std::endl;
        delete server3;
        // Continue with existing servers
    } else {
        eventLoop.addServer(server3);
    }
    */
    
    /**
     * ========================================================================
     * STEP 3: Verify we have at least one server
     * ========================================================================
     */
    if (eventLoop.getServerCount() == 0) {
        std::cerr << "Error: No servers initialized successfully" << std::endl;
        return 1;
    }
    
    std::cout << "\nSuccessfully initialized " << eventLoop.getServerCount()
              << " server(s)" << std::endl;
    
    /**
     * ========================================================================
     * STEP 4: Start the event loop
     * ========================================================================
     * 
     * This call NEVER RETURNS. The event loop runs indefinitely, processing
     * network events until the process is killed (Ctrl+C, SIGTERM, etc.).
     * 
     * WHAT HAPPENS INSIDE:
     * 1. Rebuild poll_fds array with all listening + client sockets
     * 2. Call poll() to wait for I/O activity
     * 3. Dispatch events to Server or ConnectionManager
     * 4. Repeat forever
     * 
     * TO STOP THE SERVER:
     * - Press Ctrl+C (sends SIGINT)
     * - Use kill command (sends SIGTERM)
     * - Destructors will run automatically, closing all sockets
     */
    eventLoop.run();
    
    /**
     * ========================================================================
     * This code is never reached (event loop is infinite)
     * ========================================================================
     * 
     * If we add graceful shutdown support in the future, cleanup would
     * happen here. For now, destructors handle cleanup when process exits.
     */
    
    return 0;
}