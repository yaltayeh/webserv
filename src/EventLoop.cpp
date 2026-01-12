/**
 * ============================================================================
 * EventLoop.cpp - Main Event Loop Implementation
 * ============================================================================
 * 
 * This file implements the EventLoop class methods. EventLoop is the heart
 * of the server, coordinating all I/O operations through a single poll() call.
 * 
 * ============================================================================
 */

#include "../include/EventLoop.hpp"
#include <iostream>
#include <cerrno>
#include <cstring>


EventLoop::EventLoop() {}

/**
 * Destructor - Cleans up all resources
 * 
 * DESTRUCTION ORDER (automatic, reverse of declaration):
 * 1. ~_connection_manager (Connections destroyed)
 * 2. ~_poll_fds (vector cleanup)
 * 3. ~_listening_sockets (map cleanup)
 * 4. ~_servers (manually delete Server* pointers here)
 */
EventLoop::~EventLoop()
{
    for (std::vector<Server*>::iterator it = _servers.begin();
         it != _servers.end();
         ++it)
    {
        // Delete each Server (closes listening socket in ~Server())
        delete *it;
    }
}

/**
 * addServer() - Registers a new server instance
 * 
 */
void EventLoop::addServer(Server* server)
{
    // Validate server pointer
    if (server == NULL)
        {std::cerr << "Error: Attempted to add NULL server" << std::endl; return; }
    
    // Validate server is initialized (has valid listening socket)
    int listen_fd = server->getListenFd();
    if (listen_fd < 0)
        { std::cerr << "Error: Attempted to add uninitialized server" << std::endl; return; }
    
    // Check for duplicate listening fd
    if (_listening_sockets.count(listen_fd) > 0) {
        std::cerr << "Error: Duplicate listening socket fd " << listen_fd
                  << std::endl;
        return;
    }
    
    _servers.push_back(server);
    _listening_sockets[listen_fd] = server;
}

/**
 * run() - Starts the main event loop (NEVER RETURNS)
 * 
 * WHAT THIS DOES:
 * Enters an infinite loop that polls all sockets for I/O activity and
 * dispatches events to appropriate handlers.
 * 
 * THIS FUNCTION NEVER RETURNS - it runs until the process is killed.
 */
void EventLoop::run() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Event loop starting with " << _servers.size()
              << " server(s)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    /**
     * INFINITE EVENT LOOP
     * 
     * This loop processes network events until the process is killed.
     * Standard web server behavior - run continuously, handle requests.
     * 
     * To stop the server: Ctrl+C (sends SIGINT) or kill command (SIGTERM)
     * 
     * TODO: FUTURE ENHANCEMENT:
     * Add signal handler to catch SIGINT/SIGTERM and set a "running" flag,
     * allowing graceful shutdown:
     *   while (running) { ... }
     */
    while (true) {
        
        /**
         * STEP 1: Rebuild poll_fds array from current socket state
         * Before each poll() call, we reconstruct the poll_fds array to
         * reflect all currently active sockets (listening + clients).
         */
        rebuildPollFds();
        
        /**
         * STEP 2: Wait for I/O activity on any socket
         * 
         * poll() system call monitors multiple file descriptors, waiting
         * for at least one to become ready for I/O.
         * 
         * The kernel wakes us only when I/O is ready.
         */
        int ready = poll(&_poll_fds[0], _poll_fds.size(), -1);
        
        // STEP 3: Handle poll() errors
        // NOTE: We don't check errno per 42 subject rules
        if (ready < 0)
            { std::cerr << "poll() error, continuing..." << std::endl; continue; }
        
        // STEP 4: Iterate through all sockets, checking for activity
        for (size_t i = 0; i < _poll_fds.size(); ++i) {
            
            int fd = _poll_fds[i].fd;
            short revents = _poll_fds[i].revents;
            
            // Skip if no events
            if (revents == 0) { continue; }
            
            // We need to distinguish between:
            // 1. Listening sockets (accept new connections)
            // 2. Client sockets (read/write data)

            // CASE 1: Listening socket (new connection pending)
            if (_listening_sockets.count(fd) > 0) {

                Server* server = _listening_sockets[fd];
                int client_fd = server->acceptConnection();
                
                // acceptConnection() returns -1 if accept() failed
                if (client_fd >= 0)
                    _connection_manager.addConnection(client_fd, server);
                
                continue;

            }
            
            // CASE 2: Client socket (I/O activity)
            if (_connection_manager.hasConnection(fd)) {
                // This is a client socket - check what events occurred
                
                // Sub-step 1: Check for socket errors (POLLERR, POLLHUP)
                if (!_connection_manager.handleError(fd, revents))
                    { _connection_manager.removeConnection(fd); continue; }
                
                // Sub-step 2: Handle readable socket (POLLIN)
                if (revents & POLLIN) {
                    /**
                     * Socket has data to read
                     * 
                     * handleRead() reads data, feeds to parser, generates
                     * response when request is complete.
                     */
                    if (!_connection_manager.handleRead(fd, &_poll_fds[i]))
                        { _connection_manager.removeConnection(fd); continue; }
                }
                
                // Sub-step 3: Handle writable socket (POLLOUT)
                if (revents & POLLOUT) {
                    /**
                     * Socket is ready for writing
                     * 
                     * handleWrite() writes data from write_buffer, erasing
                     * sent bytes and clearing POLLOUT when buffer is empty.
                     */
                    if (!_connection_manager.handleWrite(fd, &_poll_fds[i]))
                        { _connection_manager.removeConnection(fd); continue; }
                }
                
                continue;

            }
            
        }
        
    }
    
    /**
     * TODO: If we add graceful shutdown in the future, cleanup code would go here:
     * - Close all connections
     * - Close all listening sockets
     * - Log shutdown message
     */
}

/**
 * rebuildPollFds() - Rebuilds poll_fds array from current sockets
 * 
 * Clears poll_fds and repopulates it with:
 * 1. All listening sockets (from _listening_sockets)
 * 2. All client sockets (from _connection_manager)
 * 
 */
void EventLoop::rebuildPollFds()
{
    _poll_fds.clear();
    
    // STEP 1: Add all listening sockets
    for (std::map<int, Server*>::iterator it = _listening_sockets.begin();
         it != _listening_sockets.end();
         ++it)
    {
        pollfd pfd;
        std::memset(&pfd, 0, sizeof(pfd));
        
        pfd.fd = it->first;          /** Listening socket fd */
        pfd.events = POLLIN;         /** Monitor for new connections */
        pfd.revents = 0;             /** Will be set by poll() */
        
        _poll_fds.push_back(pfd);
    }
    
    // STEP 2: Add all client sockets
    _connection_manager.populatePollFds(_poll_fds);
}

/**
 * getServerCount() - Returns number of registered servers
 */
size_t EventLoop::getServerCount() const
{
    return _servers.size();
}
