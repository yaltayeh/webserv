#include "../include/ConnectionManager.hpp"
#include "../include/Server.hpp"
#include "../include/RequestRouter.hpp"
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
 */
void ConnectionManager::addConnection(int client_fd, Server* server)
{
    Connection* conn = new Connection();
    
    conn->fd = client_fd;
    conn->owner_server = server;
    
    _connections[client_fd] = conn;
    
    std::cout << "Added connection fd " << client_fd
              << " (total connections: " << _connections.size() << ")"
              << std::endl;
}

/**
 * removeConnection() - Removes and destroys a client connection
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
    // STEP 1: Look up connection
    Connection* conn = getConnection(fd);
    if (conn == NULL) { return false; }
    
    // STEP 2: Read data from socket into buffer
    char buffer[READ_BUFFER_SIZE];
    std::memset(buffer, 0, READ_BUFFER_SIZE); // TODO: optimize away memset
    
    ssize_t bytes_read = read(conn->fd, buffer, READ_BUFFER_SIZE);
    
    // STEP 3: Handle read() return values
    
    // Case 1: Client closed connection (graceful shutdown)
    if (bytes_read == 0) {
        std::cout << "Client fd " << conn->fd
                  << " closed connection" << std::endl;
        return false;
    }
    
    // Case 2: Read error
    if (bytes_read < 0) {
        std::cerr << "Error reading from client fd " << conn->fd << std::endl;
        return false;
    }
    
    // Case 3: Successfully read data
    std::cout << "Read " << bytes_read << " bytes from client fd "
              << conn->fd << std::endl;
    
    /**
     * STEP 4: Feed data to HTTP parser
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
        // TODO: Generate 400 Bad Request response instead of just closing
        std::cerr << "Parse error for client fd " << conn->fd << std::endl;
        return false;
    }
    
    // STEP 5: Generate response if request is complete
    if (parse_result == 0) {
        std::cout << "Request complete for client fd " << conn->fd << std::endl;
        
        // Route the request and generate response
        RequestRouter router;
        libhttp::Request request = conn->parser.getRequest();
        const ServerConfig& server_config = conn->owner_server->getConfig();
        
        libhttp::Response response = router.route(request, server_config);
        
        // Convert response to string and store in write buffer
        conn->write_buffer = response.toString();
        
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
    // STEP 1: Look up connection
    Connection* conn = getConnection(fd);
    if (conn == NULL) { return false; }
    
    // STEP 2: Check if we have data to send
    if (conn->write_buffer.empty()) {
        // clear POLLOUT to avoid unnecessary wake-ups
        poll_fd->events = POLLIN;
        return true;
    }
    
    /**
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
    
    
    // Case 1: Write error
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
    if (bytes_sent == 0) { return true; }
    
    // Case 3: Successfully wrote data
    std::cout << "Sent " << bytes_sent << " bytes to client fd "
              << conn->fd << std::endl;
    
    // STEP 5: Remove sent data from buffer
    conn->write_buffer.erase(0, bytes_sent);
    
    // STEP 6: Check if all data sent
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
 * RETURNS:
 * - true if connection is healthy (no error flags)
 * - false if connection has errors (should be closed)
 */
bool ConnectionManager::handleError(int fd, short revents)
{
    /**
     * POLLERR indicates an error condition on the socket. This is rare
     * and usually indicates a serious problem (socket closed unexpectedly,
     * network failure, etc.).
     */
    if (revents & POLLERR) {
        std::cerr << "POLLERR on socket fd " << fd << std::endl;
        return false;
    }
    
    /**
     * POLLHUP indicates the peer closed the connection (hang up). This is
     * normal behavior - clients close connections when they're done. Log
     * it but don't treat it as an error.
     */
    if (revents & POLLHUP) {
        std::cout << "POLLHUP on socket fd " << fd
                  << " (client closed connection)" << std::endl;
        return false;
    }
    
    // connection is healthy
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
 */
void ConnectionManager::populatePollFds(std::vector<pollfd>& poll_fds)
{
    // Iterate through all connections
    for (std::map<int, Connection*>::iterator it = _connections.begin();
         it != _connections.end();
         ++it)
    {
        // Create pollfd structure for this connection
        pollfd pfd;
        std::memset(&pfd, 0, sizeof(pfd));
        
        // Set file descriptor
        pfd.fd = it->second->fd;
        
        // Set events to monitor
        pfd.events = POLLIN | POLLOUT;
        
        // Initialize revents (will be set by poll())
        pfd.revents = 0;
        
        poll_fds.push_back(pfd);
    }
}

