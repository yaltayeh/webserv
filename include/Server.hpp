/**
 * Server.hpp - HTTP Server Instance Class
 * 
 * WHAT THIS CLASS DOES:
 * Represents a single HTTP server instance that listens on a specific host:port
 * pair. Each Server owns its listening socket, configuration (routes, error
 * pages, body limits), and handles accepting new client connections.
 * 
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config/ServerConfig.hpp"
#include <string>

// TODO: Forward declarations to avoid circular dependencies
class EventLoop;
class ConnectionManager;

class Server
{
public:

    Server(const ServerConfig& config);
    ~Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    
    int initialize();
    int acceptConnection();
    int getListenFd() const;
    const ServerConfig& getConfig() const;

private:
   
    ServerConfig _config;
    int _listen_fd;

};

#endif
