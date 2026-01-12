/**
 * ConnectionManager.hpp - Client Connection Management Class
 * 
 * WHAT THIS CLASS DOES:
 * Manages all active client connections in the HTTP server. Responsibilities:
 * - Storing Connection objects in a map (fd -> Connection*)
 * - Handling client socket I/O (read requests, write responses)
 * - Detecting connection errors and cleanup
 * - Associating each connection with its originating Server
 * 
 * 
 * OWNERSHIP:
 * - ConnectionManager OWNS all Connection objects (heap-allocated pointers)
 * - Connections are created in addConnection() with new
 * - Connections are destroyed in removeConnection() with delete
 * - Destructor deletes all remaining connections to prevent leaks
 * 
 */

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include "Server.hpp"
#include "../dependencies/libhttp/include/Parser.hpp"
#include <map>
#include <poll.h>
#include <string>

struct Connection {
    // Socket file descriptor for this client connection
    int fd;
    
    // Stateful HTTP parser that accumulates data across partial reads
    libhttp::Parser parser;
    
    // Response data waiting to be sent to client
    std::string write_buffer;
   
    Server* owner_server;
};

class ConnectionManager
{
public:
    
    ConnectionManager();
    ~ConnectionManager();
    ConnectionManager(const ConnectionManager& other);
    ConnectionManager& operator=(const ConnectionManager& other);
    
    void addConnection(int client_fd, Server* server);
    void removeConnection(int fd);
    bool hasConnection(int fd) const;
    Connection* getConnection(int fd);
    void populatePollFds(std::vector<pollfd>& poll_fds);
    
    bool handleRead(int fd, pollfd* poll_fd);    // POLLIN
    bool handleWrite(int fd, pollfd* poll_fd);   // POLLOUT
    bool handleError(int fd, short revents);     // POLLERR, POLLHUP
    
    size_t getConnectionCount() const;

private:

    std::map<int, Connection*> _connections;

};

#endif
