/**
 * ============================================================================
 * ConnectionManager.cpp - Client Connection Management Implementation
 * ============================================================================
 * 
 * This file implements the ConnectionManager class methods. ConnectionManager
 * owns all active client connections and handles their I/O operations.
 * 
 * ============================================================================
 */

#include "../include/ConnectionManager.hpp"
#include "../include/Server.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>

/**
 * - Matches typical memory page size on many systems
 * - Large enough to read most HTTP headers in one call
 * - Small enough to avoid excessive memory usage
 * - Powers of 2 tend to perform well with memory alignment
 */
static const size_t READ_BUFFER_SIZE = 4096; // 4 KB




ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager()
{
    for (std::map<int, Connection*>::iterator it = _connections.begin();
         it != _connections.end();
         ++it)
    {
        // Close the socket to free system resources
        // Delete the heap-allocated Connection object
        close(it->second->fd);
        delete it->second;
    }
}

/**
 * addConnection() - Adds a new client connection
 * 
 * WHAT THIS DOES:
 * Creates a new Connection object on the heap, initializes it with the
 * client socket and owning server, and stores it in the _connections map.
 * 
 * PARAMETERS:
 * - client_fd: Socket file descriptor for the new client
 * - server: Pointer to the Server that accepted this connection
 */
void ConnectionManager::addConnection(int client_fd, Server* server)
{
    Connection* conn = new Connection();
    
    /**
     * - fd: Socket file descriptor
     * - parser: Default-constructed (initial state)
     * - write_buffer: Empty string (no response data yet)
     * - owner_server: Pointer to Server for config access
     */
    conn->fd = client_fd;
    conn->owner_server = server;
    
    _connections[client_fd] = conn;
    
    std::cout << "Added connection fd " << client_fd
              << " (total connections: " << _connections.size() << ")"
              << std::endl;
}

/**
 * removeConnection() - Removes and destroys a client connection
 * 
 * WHAT THIS DOES:
 * Looks up the connection by fd, closes the socket, deletes the
 * Connection object, and removes the map entry.
 * 
 * PARAMETERS:
 * - fd: File descriptor of connection to remove
 */
void ConnectionManager::removeConnection(int fd)
{
    std::map<int, Connection*>::iterator it = _connections.find(fd);
    
    // Check if connection exists
    // defensive programming
    if (it == _connections.end()) { return; }
    
    // Get Connection pointer for convenience
    Connection* conn = it->second;
    
   
    close(conn->fd);
    
    // This calls ~Connection() which destroys the parser and write_buffer.
    delete conn;
   
    _connections.erase(it);
    
    std::cout << "Removed connection (remaining: "
              << _connections.size() << ")" << std::endl;
}

/**
 * hasConnection() - Checks if a connection exists
 */
bool ConnectionManager::hasConnection(int fd) const { return _connections.count(fd) > 0; }

/**
 * getConnection() - Retrieves a connection by fd
 * 
 * PARAMETERS:
 * - fd: File descriptor to look up
 * 
 * RETURNS:
 * - Pointer to Connection if found
 * - NULL if not found
 */
Connection* ConnectionManager::getConnection(int fd)
{
    /**
     * Look up connection in map
     */
    std::map<int, Connection*>::iterator it = _connections.find(fd);
    
    /**
     * Return pointer or NULL
     */
    if (it == _connections.end()) {
        return NULL;
    }
    
    return it->second;
}

/**
 * handleRead() - Handles readable client socket
 * 
 * WHAT THIS DOES:
 * Reads data from socket, feeds it to the HTTP parser, and generates
 * a response when the request is complete.
 * 
 * PARAMETERS:
 * - fd: File descriptor to read from
 * - poll_fd: Pointer to corresponding pollfd struct (for modifying events)
 * 
 * RETURNS:
 * - true if connection should remain open
 * - false if connection should be closed
 */
bool ConnectionManager::handleRead(int fd, pollfd* poll_fd)
{
    /**
     * ========================================================================
     * STEP 1: Look up connection
     * ========================================================================
     */
    Connection* conn = getConnection(fd);
    if (conn == NULL) {
        /**
         * Connection doesn't exist - this shouldn't happen
         * EventLoop should only call handleRead() for existing connections
         */
        std::cerr << "Error: handleRead called for non-existent fd "
                  << fd << std::endl;
        return false;
    }
    
    /**
     * ========================================================================
     * STEP 2: Read data from socket into buffer
     * ========================================================================
     * 
     * read() is a POSIX system call that reads data from a file descriptor.
     * In non-blocking mode:
     * - Returns number of bytes read (> 0) if data available
     * - Returns 0 if connection closed by peer (EOF)
     * - Returns -1 if error or no data available (EAGAIN/EWOULDBLOCK)
     * 
     * IMPORTANT: We don't check errno because 42 subject forbids it.
     * We rely on poll() to only call handleRead() when data is ready.
     */
    char buffer[READ_BUFFER_SIZE];
    std::memset(buffer, 0, READ_BUFFER_SIZE);
    
    ssize_t bytes_read = read(conn->fd, buffer, READ_BUFFER_SIZE);
    
    /**
     * ========================================================================
     * STEP 3: Handle read() return values
     * ========================================================================
     */
    
    /**
     * Case 1: Client closed connection (graceful shutdown)
     * 
     * read() returns 0 when the peer has closed the connection. This is
     * normal behavior - clients close connections after receiving responses.
     */
    if (bytes_read == 0) {
        std::cout << "Client fd " << conn->fd
                  << " closed connection" << std::endl;
        return false;
    }
    
    /**
     * Case 2: Read error
     * 
     * read() returns -1 on error. In non-blocking mode, this could be:
     * - EAGAIN/EWOULDBLOCK: No data available (shouldn't happen if poll()
     *   indicated POLLIN, but possible in rare race conditions)
     * - Other errors: Socket broken, network issue, etc.
     * 
     * We treat all -1 returns as fatal errors and close the connection.
     * More sophisticated error handling could distinguish error types.
     */
    if (bytes_read < 0) {
        std::cerr << "Error reading from client fd " << conn->fd << std::endl;
        return false;
    }
    
    /**
     * Case 3: Successfully read data
     */
    std::cout << "Read " << bytes_read << " bytes from client fd "
              << conn->fd << std::endl;
    
    /**
     * ========================================================================
     * STEP 4: Feed data to HTTP parser
     * ========================================================================
     * 
     * The parser is stateful and accumulates data across multiple calls.
     * 
     * parser.parse() returns:
     * - 0: Request complete (all headers and body received)
     * - 1: Request incomplete (need more data)
     * - -1: Parse error (malformed HTTP request)
     * 
     * INCREMENTAL PARSING:
     * HTTP requests might arrive fragmented across multiple TCP packets:
     *   Packet 1: "GET / HT"
     *   Packet 2: "TP/1.0\r\nHost: "
     *   Packet 3: "localhost\r\n\r\n"
     * 
     * Parser handles this seamlessly by maintaining internal state.
     */
    int parse_result = conn->parser.parse(buffer, bytes_read);
    
    if (parse_result == -1) {
        /**
         * Parse error: Malformed HTTP request
         * 
         * TODO: Generate 400 Bad Request response instead of just closing
         */
        std::cerr << "Parse error for client fd " << conn->fd << std::endl;
        return false;
    }
    
    /**
     * ========================================================================
     * STEP 5: Generate response if request is complete
     * ========================================================================
     */
    if (parse_result == 0) {
        std::cout << "Request complete for client fd " << conn->fd << std::endl;
        
        /**
         * TODO: Replace this hardcoded response with proper request routing
         * 
         * FUTURE IMPLEMENTATION:
         * 1. Get ServerConfig from conn->owner_server->getConfig()
         * 2. Extract request URI from conn->parser.getRequest()
         * 3. Match URI against routes in ServerConfig
         * 4. Handle based on route type:
         *    - Static file serving
         *    - CGI execution
         *    - Redirect
         *    - Error page
         * 5. Generate appropriate HTTP response
         * 
         * For now, we return a simple "Hello, World!" response.
         */
        conn->write_buffer =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";
        
        /**
         * Enable POLLOUT event to get notified when socket is writable
         * 
         * We keep POLLIN enabled too in case client sends more data
         * (though HTTP/1.0 typically closes after one request/response).
         */
        poll_fd->events = POLLIN | POLLOUT;
        
        std::cout << "Response ready, waiting for socket to be writable"
                  << std::endl;
    }
    
    /**
     * parse_result == 1 means incomplete request, keep waiting for more data
     * Connection remains open, POLLIN event stays enabled
     */
    
    return true;
}

/**
 * handleWrite() - Handles writable client socket
 * 
 * WHAT THIS DOES:
 * Writes data from Connection::write_buffer to the socket, erasing
 * successfully written bytes. Clears POLLOUT flag when buffer is empty.
 * 
 * PARAMETERS:
 * - fd: File descriptor to write to
 * - poll_fd: Pointer to corresponding pollfd struct (for modifying events)
 * 
 * RETURNS:
 * - true if connection should remain open
 * - false if write error occurred
 */
bool ConnectionManager::handleWrite(int fd, pollfd* poll_fd)
{
    /**
     * ========================================================================
     * STEP 1: Look up connection
     * ========================================================================
     */
    Connection* conn = getConnection(fd);
    if (conn == NULL) {
        std::cerr << "Error: handleWrite called for non-existent fd "
                  << fd << std::endl;
        return false;
    }
    
    /**
     * ========================================================================
     * STEP 2: Check if we have data to send
     * ========================================================================
     * 
     * If write_buffer is empty, there's nothing to write. This shouldn't
     * happen because we only enable POLLOUT when we have data, but check
     * defensively anyway.
     */
    if (conn->write_buffer.empty()) {
        /**
         * No data to write - clear POLLOUT to avoid unnecessary wake-ups
         */
        poll_fd->events = POLLIN;
        return true;
    }
    
    /**
     * ========================================================================
     * STEP 3: Write data to socket
     * ========================================================================
     * 
     * write() is a POSIX system call that writes data to a file descriptor.
     * In non-blocking mode:
     * - Returns number of bytes written (> 0) if socket has buffer space
     * - Returns 0 if nothing written (rare, treat as error)
     * - Returns -1 if error or socket not ready (EAGAIN/EWOULDBLOCK)
     * 
     * PARTIAL WRITES:
     * write() might not send all data in one call if the TCP send buffer
     * is full. We must track how many bytes were sent and retry later for
     * the remainder. This is fundamental to non-blocking I/O.
     * 
     * Example: write_buffer contains 1000 bytes, but write() only sends 300.
     * We erase the first 300 bytes and leave 700 in the buffer. Next time
     * poll() indicates POLLOUT, we try again with the remaining 700 bytes.
     */
    ssize_t bytes_sent = write(conn->fd,
                               conn->write_buffer.c_str(),
                               conn->write_buffer.size());
    
    /**
     * ========================================================================
     * STEP 4: Handle write() return values
     * ========================================================================
     */
    
    /**
     * Case 1: Write error
     */
    if (bytes_sent < 0) {
        /**
         * write() returned -1, indicating error. Possible causes:
         * - EAGAIN/EWOULDBLOCK: Socket not ready (shouldn't happen if poll()
         *   indicated POLLOUT, but theoretically possible)
         * - EPIPE: Connection closed by peer (broken pipe)
         * - Other errors: Network issue, socket broken, etc.
         * 
         * We treat all errors as fatal and close the connection.
         */
        std::cerr << "Error writing to client fd " << conn->fd << std::endl;
        return false;
    }
    
    /**
     * Case 2: Nothing written
     * 
     * write() returned 0, which means no bytes were sent. This is unusual
     * but not necessarily an error. Treat conservatively as an error.
     */
    if (bytes_sent == 0) {
        /**
         * Retry on next POLLOUT event
         * 
         * Alternatively, we could treat this as an error. For now, keep
         * connection alive and hope the next write() succeeds.
         */
        return true;
    }
    
    /**
     * Case 3: Successfully wrote data
     */
    std::cout << "Sent " << bytes_sent << " bytes to client fd "
              << conn->fd << std::endl;
    
    /**
     * ========================================================================
     * STEP 5: Remove sent data from buffer
     * ========================================================================
     * 
     * std::string::erase(pos, count) removes characters starting at pos.
     * We erase from position 0 to bytes_sent, removing the sent data.
     * 
     * Example:
     * - Before: write_buffer = "HTTP/1.0 200 OK\r\n..."
     * - bytes_sent = 10
     * - After: write_buffer = "200 OK\r\n..."
     */
    conn->write_buffer.erase(0, bytes_sent);
    
    /**
     * ========================================================================
     * STEP 6: Check if all data sent
     * ========================================================================
     */
    if (conn->write_buffer.empty()) {
        std::cout << "All data sent to client fd " << conn->fd << std::endl;
        
        /**
         * Clear POLLOUT event to stop being notified of writeability
         * 
         * CPU OPTIMIZATION:
         * Sockets are almost always writable (TCP send buffer has space).
         * If we keep POLLOUT enabled, poll() wakes us up every iteration
         * saying "socket is writable!" even though we have nothing to write.
         * This wastes CPU cycles. By clearing POLLOUT when buffer is empty,
         * we only get notified when we actually have data to send.
         */
        poll_fd->events = POLLIN;
        
        /**
         * TODO: For HTTP/1.0, close connection after sending response
         * 
         * HTTP/1.0 semantics: One request per connection, then close.
         * HTTP/1.1 supports Keep-Alive (persistent connections), but we're
         * implementing HTTP/1.0 for simplicity.
         * 
         * For now, we keep the connection open. Future enhancement:
         *   return false;  // Close connection after response sent
         */
    }
    
    return true;
}

/**
 * handleError() - Handles socket errors and hangups
 * 
 * WHAT THIS DOES:
 * Checks if POLLERR or POLLHUP flags are set in revents, indicating
 * the connection is broken and should be closed.
 * 
 * PARAMETERS:
 * - fd: File descriptor (used for logging)
 * - revents: The revents field from pollfd struct
 * 
 * RETURNS:
 * - true if connection is healthy (no error flags)
 * - false if connection has errors (should be closed)
 */
bool ConnectionManager::handleError(int fd, short revents)
{
    /**
     * Check for POLLERR flag
     * 
     * POLLERR indicates an error condition on the socket. This is rare
     * and usually indicates a serious problem (socket closed unexpectedly,
     * network failure, etc.).
     */
    if (revents & POLLERR) {
        std::cerr << "POLLERR on socket fd " << fd << std::endl;
        return false;
    }
    
    /**
     * Check for POLLHUP flag
     * 
     * POLLHUP indicates the peer closed the connection (hang up). This is
     * normal behavior - clients close connections when they're done. Log
     * it but don't treat it as an error.
     */
    if (revents & POLLHUP) {
        std::cout << "POLLHUP on socket fd " << fd
                  << " (client closed connection)" << std::endl;
        return false;
    }
    
    /**
     * No error flags set - connection is healthy
     */
    return true;
}

/**
 * getConnectionCount() - Returns number of active connections
 */
size_t ConnectionManager::getConnectionCount() const
{
    return _connections.size();
}

/**
 * populatePollFds() - Adds all client sockets to poll_fds vector
 * 
 * WHAT THIS DOES:
 * Iterates through _connections map and creates a pollfd structure
 * for each client socket, adding it to the provided vector.
 * 
 * PARAMETERS:
 * - poll_fds: Reference to vector where pollfd structures are added
 */
void ConnectionManager::populatePollFds(std::vector<pollfd>& poll_fds)
{
    /**
     * Iterate through all connections
     */
    for (std::map<int, Connection*>::iterator it = _connections.begin();
         it != _connections.end();
         ++it)
    {
        /**
         * Create pollfd structure for this connection
         */
        pollfd pfd;
        std::memset(&pfd, 0, sizeof(pfd));
        
        /**
         * Set file descriptor
         */
        pfd.fd = it->second->fd;
        
        /**
         * Set events to monitor
         * 
         * POLLIN: Notify when data available to read
         * POLLOUT: Notify when socket ready for writing
         * 
         * We monitor both events. POLLOUT will be cleared by handleWrite()
         * when write_buffer is empty (CPU optimization).
         */
        pfd.events = POLLIN | POLLOUT;
        
        /**
         * Initialize revents (will be set by poll())
         */
        pfd.revents = 0;
        
        /**
         * Add to poll_fds vector
         */
        poll_fds.push_back(pfd);
    }
}

