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
        /**
         * Delete each Server (closes listening socket in ~Server())
         */
        delete *it;
    }
}

/**
 * addServer() - Registers a new server instance
 * 
 * WHAT THIS DOES:
 * Takes ownership of a Server pointer, extracts its listening socket fd,
 * and stores both in _servers vector and _listening_sockets map.
 * 
 * PARAMETERS:
 * - server: Pointer to initialized Server (ownership transferred)
 */
void EventLoop::addServer(Server* server)
{
    /**
     * Validate server pointer
     */
    if (server == NULL) {
        std::cerr << "Error: Attempted to add NULL server" << std::endl;
        return;
    }
    
    /**
     * Validate server is initialized (has valid listening socket)
     */
    int listen_fd = server->getListenFd();
    if (listen_fd < 0) {
        std::cerr << "Error: Attempted to add uninitialized server" << std::endl;
        /**
         * Don't take ownership of broken server - caller should delete it
         */
        return;
    }
    
    /**
     * Check for duplicate listening fd
     * 
     * This would indicate two servers trying to bind the same port,
     * which should have failed during server->initialize().
     * Check defensively anyway.
     */
    if (_listening_sockets.count(listen_fd) > 0) {
        std::cerr << "Error: Duplicate listening socket fd " << listen_fd
                  << std::endl;
        return;
    }
    
    /**
     * Add to _servers vector for iteration and cleanup
     */
    _servers.push_back(server);
    
    /**
     * Add to _listening_sockets map for fast lookup during dispatch
     */
    _listening_sockets[listen_fd] = server;
    
    const ServerConfig& cfg = server->getConfig();
    std::cout << "Added server on " << cfg.host << ":" << cfg.port
              << " to EventLoop (fd " << listen_fd << ")" << std::endl;
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
void EventLoop::run()
{
    /**
     * Validate we have at least one server
     */
    if (_servers.empty()) {
        std::cerr << "Error: No servers registered, cannot start event loop"
                  << std::endl;
        return;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Event loop starting with " << _servers.size()
              << " server(s)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    /**
     * ========================================================================
     * INFINITE EVENT LOOP
     * ========================================================================
     * 
     * This loop processes network events until the process is killed.
     * Standard web server behavior - run continuously, handle requests.
     * 
     * To stop the server: Ctrl+C (sends SIGINT) or kill command (SIGTERM)
     * 
     * FUTURE ENHANCEMENT:
     * Add signal handler to catch SIGINT/SIGTERM and set a "running" flag,
     * allowing graceful shutdown:
     *   while (running) { ... }
     */
    while (true) {
        
        /**
         * ====================================================================
         * STEP 1: Rebuild poll_fds array from current socket state
         * ====================================================================
         * 
         * Before each poll() call, we reconstruct the poll_fds array to
         * reflect all currently active sockets (listening + clients).
         * 
         * WHY REBUILD:
         * Simpler than maintaining indices as connections come and go.
         * Performance impact is negligible for typical server loads.
         */
        rebuildPollFds();
        
        /**
         * ====================================================================
         * STEP 2: Wait for I/O activity on any socket
         * ====================================================================
         * 
         * poll() system call monitors multiple file descriptors, waiting
         * for at least one to become ready for I/O.
         * 
         * PARAMETERS:
         * - &_poll_fds[0]: Pointer to array of pollfd structures
         * - _poll_fds.size(): Number of file descriptors to monitor
         * - -1: Timeout in milliseconds (-1 = infinite, block until activity)
         * 
         * RETURNS:
         * - > 0: Number of file descriptors with events (ready for I/O)
         * - 0: Timeout expired (not possible with timeout = -1)
         * - -1: Error occurred (errno set)
         * 
         * WHAT poll() DOES:
         * 1. Blocks (sleeps) until at least one fd has activity
         * 2. Sets revents field in pollfd structures for ready fds
         * 3. Returns control to our program
         * 4. We iterate poll_fds checking revents to see what happened
         * 
         * CPU EFFICIENCY:
         * With timeout = -1, our process SLEEPS when there's no activity.
         * No CPU usage while idle! This is far superior to busy-waiting
         * or short timeouts. The kernel wakes us only when I/O is ready.
         */
        int ready = poll(&_poll_fds[0], _poll_fds.size(), -1);
        
        /**
         * ====================================================================
         * STEP 3: Handle poll() errors
         * ====================================================================
         */
        if (ready < 0) {
            /**
             * poll() returned -1, indicating an error
             * 
             * Common causes:
             * - EINTR: Interrupted by signal (e.g., SIGINT from Ctrl+C)
             * - ENOMEM: Out of memory
             * - EINVAL: Invalid arguments
             * 
             * DECISION: Continue running and hope error was transient.
             * Alternative: Exit gracefully on error.
             * 
             * NOTE: We don't check errno per 42 subject rules, but errno
             * would tell us the specific error.
             */
            std::cerr << "poll() error, continuing..." << std::endl;
            continue;
        }
        
        /**
         * ====================================================================
         * STEP 4: Iterate through all sockets, checking for activity
         * ====================================================================
         * 
         * poll() sets revents field for sockets that have activity.
         * We iterate all pollfd structures, checking revents to see
         * what events occurred.
         */
        for (size_t i = 0; i < _poll_fds.size(); ++i) {
            
            /**
             * Extract fd and revents for readability
             */
            int fd = _poll_fds[i].fd;
            short revents = _poll_fds[i].revents;
            
            /**
             * Skip sockets with no events
             * 
             * poll() only sets revents for sockets with activity.
             * For all other sockets, revents is 0.
             */
            if (revents == 0) {
                continue;
            }
            
            /**
             * ================================================================
             * DISPATCH: Determine socket type and call appropriate handler
             * ================================================================
             * 
             * We need to distinguish between:
             * 1. Listening sockets (accept new connections)
             * 2. Client sockets (read/write data)
             * 
             * STRATEGY:
             * Check _listening_sockets map first. If fd is a listening
             * socket, handle new connection. Otherwise, check if fd is
             * a client connection and handle I/O.
             */
            
            /**
             * ================================================================
             * CASE 1: Listening socket (new connection pending)
             * ================================================================
             */
            if (_listening_sockets.count(fd) > 0) {
                /**
                 * This is a listening socket - new client wants to connect
                 * 
                 * Retrieve the Server that owns this listening socket
                 */
                Server* server = _listening_sockets[fd];
                
                /**
                 * Accept the new connection
                 * 
                 * server->acceptConnection() calls accept() on the listening
                 * socket, creating a new client socket and returning its fd.
                 */
                int client_fd = server->acceptConnection();
                
                /**
                 * Check if accept succeeded
                 * 
                 * acceptConnection() returns -1 if accept() failed or no
                 * connections are pending (shouldn't happen if poll() said
                 * POLLIN, but check defensively).
                 */
                if (client_fd >= 0) {
                    /**
                     * Add new client to ConnectionManager
                     * 
                     * This creates a Connection object, associates it with
                     * the Server, and stores it in the connections map.
                     */
                    _connection_manager.addConnection(client_fd, server);
                }
                
                /**
                 * Continue to next socket
                 * 
                 * We've handled this socket's event, move on.
                 */
                continue;
            }
            
            /**
             * ================================================================
             * CASE 2: Client socket (I/O activity)
             * ================================================================
             */
            if (_connection_manager.hasConnection(fd)) {
                /**
                 * This is a client socket - check what events occurred
                 * 
                 * Possible events:
                 * - POLLIN: Data available to read
                 * - POLLOUT: Socket ready for writing
                 * - POLLERR: Socket error
                 * - POLLHUP: Connection closed by peer
                 * 
                 * PROCESSING ORDER:
                 * 1. Check for errors first (POLLERR, POLLHUP)
                 * 2. Handle reads (POLLIN)
                 * 3. Handle writes (POLLOUT)
                 * 
                 * If any handler returns false, remove the connection.
                 */
                
                bool keep_alive = true;
                
                /**
                 * ------------------------------------------------------------
                 * Sub-step 1: Check for socket errors
                 * ------------------------------------------------------------
                 */
                if (!_connection_manager.handleError(fd, revents)) {
                    /**
                     * Error detected (POLLERR or POLLHUP)
                     * Connection should be closed
                     */
                    keep_alive = false;
                }
                
                /**
                 * ------------------------------------------------------------
                 * Sub-step 2: Handle readable socket (POLLIN)
                 * ------------------------------------------------------------
                 */
                if (keep_alive && (revents & POLLIN)) {
                    /**
                     * Socket has data to read
                     * 
                     * handleRead() reads data, feeds to parser, generates
                     * response when request is complete.
                     */
                    if (!_connection_manager.handleRead(fd, &_poll_fds[i])) {
                        /**
                         * Read failed or client disconnected
                         * Connection should be closed
                         */
                        keep_alive = false;
                    }
                }
                
                /**
                 * ------------------------------------------------------------
                 * Sub-step 3: Handle writable socket (POLLOUT)
                 * ------------------------------------------------------------
                 * 
                 * IMPORTANT: Only handle POLLOUT if connection is still alive
                 * after handleRead(). If client disconnected during read,
                 * don't try to write.
                 */
                if (keep_alive && (revents & POLLOUT)) {
                    /**
                     * Socket is ready for writing
                     * 
                     * handleWrite() writes data from write_buffer, erasing
                     * sent bytes and clearing POLLOUT when buffer is empty.
                     */
                    if (!_connection_manager.handleWrite(fd, &_poll_fds[i])) {
                        /**
                         * Write failed
                         * Connection should be closed
                         */
                        keep_alive = false;
                    }
                }
                
                /**
                 * ------------------------------------------------------------
                 * Sub-step 4: Remove connection if needed
                 * ------------------------------------------------------------
                 */
                if (!keep_alive) {
                    /**
                     * One of the handlers returned false, indicating the
                     * connection should be closed.
                     * 
                     * removeConnection() closes the socket, deletes the
                     * Connection object, and removes it from the map.
                     */
                    _connection_manager.removeConnection(fd);
                }
                
                /**
                 * Continue to next socket
                 */
                continue;
            }
            
            /**
             * ================================================================
             * CASE 3: Unknown socket (shouldn't happen)
             * ================================================================
             * 
             * If we reach here, fd is neither a listening socket nor a
             * client connection. This indicates a bug in our logic.
             */
            std::cerr << "Warning: Unknown socket fd " << fd
                      << " in poll_fds" << std::endl;
        }
        
        /**
         * ====================================================================
         * End of poll() iteration - loop continues indefinitely
         * ====================================================================
         */
    }
    
    /**
     * This line is never reached (infinite loop above)
     * 
     * If we add graceful shutdown in the future, cleanup code would go here:
     * - Close all connections
     * - Close all listening sockets
     * - Log shutdown message
     */
}

/**
 * rebuildPollFds() - Rebuilds poll_fds array from current sockets
 * 
 * WHAT THIS DOES:
 * Clears poll_fds and repopulates it with:
 * 1. All listening sockets (from _listening_sockets)
 * 2. All client sockets (from _connection_manager)
 * 
 * WHY THIS APPROACH:
 * Simpler than maintaining indices as connections are added/removed.
 * Performance is fine for typical server loads (hundreds of connections).
 */
void EventLoop::rebuildPollFds()
{
    /**
     * Clear existing poll_fds array
     */
    _poll_fds.clear();
    
    /**
     * ========================================================================
     * STEP 1: Add all listening sockets
     * ========================================================================
     * 
     * Iterate _listening_sockets map and add each fd to poll_fds.
     * 
     * EVENTS FOR LISTENING SOCKETS:
     * - POLLIN: New connection pending (client trying to connect)
     * 
     * We don't need POLLOUT for listening sockets - they're never written to.
     */
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
    
    /**
     * ========================================================================
     * STEP 2: Add all client sockets
     * ========================================================================
     * 
     * ConnectionManager iterates its internal _connections map and adds
     * each client socket to poll_fds with appropriate events (POLLIN | POLLOUT).
     * 
     * This is cleaner than exposing ConnectionManager's internal map or
     * making EventLoop a friend class. Encapsulation preserved!
     */
    _connection_manager.populatePollFds(_poll_fds);
}

/**
 * getServerCount() - Returns number of registered servers
 * 
 * RETURNS:
 * Number of Server instances in _servers vector
 */
size_t EventLoop::getServerCount() const
{
    return _servers.size();
}
