/**
 * ============================================================================
 * ConnectionManager.hpp - Client Connection Management Class
 * ============================================================================
 * 
 * WHAT THIS CLASS DOES:
 * Manages all active client connections in the HTTP server. Responsibilities:
 * - Storing Connection objects in a map (fd -> Connection*)
 * - Handling client socket I/O (read requests, write responses)
 * - Detecting connection errors and cleanup
 * - Associating each connection with its originating Server
 * 
 * WHY THIS CLASS EXISTS:
 * Separates connection management logic from the main event loop and server
 * listening logic. This follows Single Responsibility Principle: Server class
 * handles accepting connections, ConnectionManager handles client I/O.
 * 
 * DESIGN PATTERN:
 * Uses std::map<int, Connection*> for O(log n) lookup by file descriptor.
 * This is cleaner than parallel vectors and makes dispatching straightforward:
 * 
 *   if (connectionManager.hasConnection(fd)) {
 *       connectionManager.handleRead(fd, poll_fd);
 *   }
 * 
 * OWNERSHIP:
 * - ConnectionManager OWNS all Connection objects (heap-allocated pointers)
 * - Connections are created in addConnection() with new
 * - Connections are destroyed in removeConnection() with delete
 * - Destructor deletes all remaining connections to prevent leaks
 * 
 * INTEGRATION WITH EVENT LOOP:
 * EventLoop owns a single ConnectionManager instance. When poll() detects
 * activity on a client socket:
 * 1. EventLoop looks up fd in ConnectionManager
 * 2. EventLoop calls handleRead() or handleWrite() based on revents flags
 * 3. If handler returns false, EventLoop calls removeConnection()
 * 
 * ============================================================================
 */

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include "Server.hpp"
#include "../dependencies/libhttp/include/Parser.hpp"
#include <map>
#include <poll.h>
#include <string>

/**
 * ============================================================================
 * Connection Structure - Represents a single client connection
 * ============================================================================
 * 
 * WHAT THIS STORES:
 * All state needed to handle a connected HTTP client:
 * - Socket file descriptor for I/O operations
 * - HTTP parser that accumulates data across multiple read() calls
 * - Write buffer containing response data waiting to be sent
 * - Pointer to the Server that accepted this connection (for config access)
 * 
 * WHY owner_server POINTER:
 * When handling a request, we need to know which server's configuration
 * applies (routes, error pages, body limits). The owner_server pointer
 * provides this association without copying entire ServerConfig objects.
 * 
 * LIFECYCLE:
 * 1. Created by ConnectionManager::addConnection() when Server accepts client
 * 2. Registered in ConnectionManager's internal map: fd -> Connection*
 * 3. Updated as data is read/written over multiple poll() iterations
 * 4. Destroyed by ConnectionManager::removeConnection() when client disconnects
 * 
 * ============================================================================
 */
struct Connection {
    /**
     * Socket file descriptor for this client connection
     * 
     * Valid values: >= 0 (valid socket fd)
     * 
     * This fd is:
     * - Obtained from Server::acceptConnection()
     * - Set to non-blocking mode immediately after accept
     * - Registered in EventLoop's poll_fds array (POLLIN | POLLOUT)
     * - Closed when connection is removed
     */
    int fd;
    
    /**
     * Stateful HTTP parser that accumulates data across partial reads
     * 
     * The parser handles TCP stream fragmentation (HTTP request might arrive
     * in multiple chunks). It maintains internal state (REQUEST_LINE ->
     * HEADERS -> BODY -> COMPLETE) and returns:
     * - 0 when request is complete
     * - 1 when more data is needed
     * - -1 on parsing error (malformed HTTP)
     */
    libhttp::Parser parser;
    
    /**
     * Response data waiting to be sent to client
     * 
     * This buffer stores the HTTP response (headers + body). Writing might
     * not complete in one send() call (TCP buffer full), so data is erased
     * from the front as it's successfully written.
     * 
     * When buffer becomes empty, POLLOUT flag is removed to avoid unnecessary
     * wake-ups (CPU optimization - no need to check writability when there's
     * nothing to write).
     */
    std::string write_buffer;
    
    /**
     * Pointer to the Server that accepted this connection
     * 
     * WHY THIS IS NEEDED:
     * Multiple servers can run simultaneously (different ports/configs). When
     * processing a request, we need to know which server's configuration to
     * use for routing, error pages, CGI execution, etc.
     * 
     * LIFETIME: This pointer remains valid as long as the Server exists in
     * EventLoop. Since EventLoop owns all Servers, and Connections are owned
     * by ConnectionManager within EventLoop, this pointer is always valid.
     * 
     * USAGE EXAMPLE:
     *   const ServerConfig& cfg = conn->owner_server->getConfig();
     *   // Use cfg.routes, cfg.error_pages, cfg.client_max_body_size, etc.
     */
    Server* owner_server;
};

/**
 * ============================================================================
 * ConnectionManager Class - Manages All Client Connections
 * ============================================================================
 */
class ConnectionManager
{
public:
    /**
     * ========================================================================
     * Constructor - Creates empty ConnectionManager
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Initializes an empty connection map. No connections exist until
     * addConnection() is called.
     */
    ConnectionManager();
    
    /**
     * ========================================================================
     * Destructor - Cleans up all connections
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Iterates through all connections, closes their sockets, and deletes
     * the Connection objects. This prevents memory leaks and ensures clean
     * shutdown.
     * 
     * WHY THIS IS IMPORTANT:
     * RAII principle: Resources acquired during lifetime (Connection objects,
     * open sockets) must be released automatically. Even if the main program
     * exits abruptly, the destructor ensures cleanup.
     */
    ~ConnectionManager();
    
    /**
     * ========================================================================
     * Copy Constructor - DISABLED (private)
     * ========================================================================
     * 
     * WHY COPYING IS DISABLED:
     * ConnectionManager manages dynamic memory (Connection pointers) and
     * system resources (file descriptors). Shallow copying would create
     * double-free bugs. Deep copying would duplicate file descriptors,
     * which is nonsensical (same fd can't be used by two objects safely).
     * 
     * BEST PRACTICE: Make ConnectionManager non-copyable by declaring copy
     * constructor and assignment operator as private with no implementation.
     */
    ConnectionManager(const ConnectionManager& other);
    
    /**
     * ========================================================================
     * Assignment Operator - DISABLED (private)
     * ========================================================================
     * 
     * Same rationale as copy constructor: assigning ConnectionManagers would
     * cause resource management nightmares.
     */
    ConnectionManager& operator=(const ConnectionManager& other);
    
    /**
     * ========================================================================
     * addConnection() - Adds a new client connection
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Creates a new Connection object on the heap (with new)
     * 2. Initializes its fd, parser, write_buffer, and owner_server pointer
     * 3. Inserts into the _connections map: fd -> Connection*
     * 
     * WHY HEAP ALLOCATION:
     * Connection objects must remain at stable memory addresses because we
     * store pointers to them. Stack allocation or std::vector would cause
     * pointer invalidation when resizing. std::map guarantees pointer stability.
     * 
     * PARAMETERS:
     * - client_fd: File descriptor for the new client socket (from accept())
     * - server: Pointer to the Server that accepted this connection
     *           (determines which config to use for request handling)
     * 
     * PRECONDITIONS:
     * - client_fd must be a valid, non-blocking socket
     * - client_fd must not already exist in the map (duplicate would be a bug)
     * - server must be a valid pointer (owned by EventLoop)
     * 
     * EXAMPLE USAGE (from EventLoop):
     *   int client_fd = server->acceptConnection();
     *   if (client_fd != -1) {
     *       connectionManager.addConnection(client_fd, server);
     *   }
     */
    void addConnection(int client_fd, Server* server);
    
    /**
     * ========================================================================
     * removeConnection() - Removes and destroys a client connection
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Looks up the Connection by fd in _connections map
     * 2. Closes the socket (close() system call)
     * 3. Deletes the Connection object (frees memory)
     * 4. Erases the map entry
     * 
     * WHY SOCKET IS CLOSED HERE:
     * The socket is opened by Server::acceptConnection(), but ownership is
     * transferred to ConnectionManager. Whoever creates a resource is not
     * necessarily who destroys it - ConnectionManager owns the Connection
     * lifecycle, so it closes the socket.
     * 
     * PARAMETERS:
     * - fd: File descriptor of the connection to remove
     * 
     * POSTCONDITIONS:
     * - Socket is closed (no longer valid)
     * - Memory is freed (Connection* pointer is deleted)
     * - Map entry is removed
     * - If fd doesn't exist in map, function does nothing (safe)
     * 
     * EXAMPLE USAGE (from EventLoop):
     *   if (!connectionManager.handleRead(fd, poll_fd)) {
     *       connectionManager.removeConnection(fd);
     *   }
     */
    void removeConnection(int fd);
    
    /**
     * ========================================================================
     * hasConnection() - Checks if a connection exists
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns true if the given fd exists in the _connections map, false
     * otherwise. This is used by EventLoop to distinguish between listening
     * sockets (handled by Server) and client sockets (handled here).
     * 
     * PARAMETERS:
     * - fd: File descriptor to check
     * 
     * RETURNS:
     * - true if fd is a client connection
     * - false if fd is not in the map
     * 
     * USAGE BY EVENT LOOP:
     * After poll() detects activity on fd:
     *   if (listening_sockets.count(fd)) {
     *       // Handle new connection
     *   } else if (connectionManager.hasConnection(fd)) {
     *       // Handle client I/O
     *   }
     */
    bool hasConnection(int fd) const;
    
    /**
     * ========================================================================
     * getConnection() - Retrieves a connection by fd
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns a pointer to the Connection object associated with the given
     * file descriptor. Returns NULL if fd doesn't exist in the map.
     * 
     * WHY THIS IS USEFUL:
     * Sometimes EventLoop needs direct access to the Connection (e.g., to
     * inspect parser state or modify write_buffer) without calling a handler
     * method.
     * 
     * PARAMETERS:
     * - fd: File descriptor to look up
     * 
     * RETURNS:
     * - Pointer to Connection object if found
     * - NULL if not found
     * 
     * WARNING: Returned pointer is owned by ConnectionManager. Do not delete it!
     */
    Connection* getConnection(int fd);
    
    /**
     * ========================================================================
     * handleRead() - Handles readable client socket
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Reads data from socket into a buffer (up to 4096 bytes)
     * 2. Feeds data to the Connection's HTTP parser
     * 3. If parser returns complete (0), generates HTTP response
     * 4. If parser returns incomplete (1), waits for more data
     * 5. If parser returns error (-1), closes connection
     * 
     * WHY THIS DESIGN:
     * HTTP requests can arrive in multiple TCP packets (fragmentation). The
     * parser accumulates data across multiple read() calls until the full
     * request is received. This is the core of non-blocking HTTP handling.
     * 
     * PARAMETERS:
     * - fd: File descriptor to read from
     * - poll_fd: Pointer to corresponding pollfd struct (to modify events)
     * 
     * RETURNS:
     * - true if connection should remain open
     * - false if connection should be closed (error or client disconnect)
     * 
     * SIDE EFFECTS:
     * - Modifies Connection::parser state
     * - Sets Connection::write_buffer with response data
     * - Sets POLLOUT flag on poll_fd when response is ready
     * 
     * ERROR HANDLING:
     * - read() returns 0: Client closed connection (return false)
     * - read() returns -1 with EAGAIN/EWOULDBLOCK: No data yet (return true)
     * - read() returns -1 with other errno: Socket error (return false)
     * - Parser returns -1: Malformed HTTP request (return false)
     * 
     * EXAMPLE USAGE (from EventLoop):
     *   if (poll_fds[i].revents & POLLIN) {
     *       if (!connectionManager.handleRead(fd, &poll_fds[i])) {
     *           connectionManager.removeConnection(fd);
     *       }
     *   }
     */
    bool handleRead(int fd, pollfd* poll_fd);
    
    /**
     * ========================================================================
     * handleWrite() - Handles writable client socket
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Writes data from Connection::write_buffer to socket
     * 2. Erases successfully written bytes from the front of the buffer
     * 3. If buffer becomes empty, removes POLLOUT flag (CPU optimization)
     * 
     * WHY INCREMENTAL WRITING:
     * write() might not send all data in one call if the TCP send buffer is
     * full. We must track how many bytes were sent and try again later for
     * the remainder. This is fundamental to non-blocking I/O.
     * 
     * PARAMETERS:
     * - fd: File descriptor to write to
     * - poll_fd: Pointer to corresponding pollfd struct (to modify events)
     * 
     * RETURNS:
     * - true if connection should remain open
     * - false if write error occurred (socket broken)
     * 
     * SIDE EFFECTS:
     * - Erases sent bytes from Connection::write_buffer
     * - Clears POLLOUT flag when buffer is empty
     * 
     * ERROR HANDLING:
     * - write() returns -1 with EAGAIN/EWOULDBLOCK: Socket not ready (return true)
     * - write() returns -1 with other errno: Socket error (return false)
     * 
     * CPU OPTIMIZATION:
     * When write_buffer is empty, we clear POLLOUT flag. Otherwise, poll()
     * would wake us up every iteration saying "socket is writable!" even
     * though we have nothing to write. This wastes CPU cycles.
     * 
     * EXAMPLE USAGE (from EventLoop):
     *   if (poll_fds[i].revents & POLLOUT) {
     *       if (!connectionManager.handleWrite(fd, &poll_fds[i])) {
     *           connectionManager.removeConnection(fd);
     *       }
     *   }
     */
    bool handleWrite(int fd, pollfd* poll_fd);
    
    /**
     * ========================================================================
     * handleError() - Handles socket errors and hangups
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Checks if POLLERR (socket error) or POLLHUP (client disconnected)
     * flags are set in revents. These indicate the connection is broken
     * and should be closed immediately.
     * 
     * WHY THIS IS A SEPARATE METHOD:
     * Error checking logic should happen before read/write attempts. If
     * POLLERR or POLLHUP is set, the socket is already broken - attempting
     * read/write would fail anyway. Checking errors first is cleaner.
     * 
     * PARAMETERS:
     * - fd: File descriptor to check (used for logging, not lookup)
     * - revents: The revents field from pollfd struct
     * 
     * RETURNS:
     * - true if connection is healthy (no error flags set)
     * - false if connection has errors (POLLERR or POLLHUP set)
     * 
     * EXAMPLE USAGE (from EventLoop):
     *   if (!connectionManager.handleError(fd, poll_fds[i].revents)) {
     *       connectionManager.removeConnection(fd);
     *       continue;  // Skip read/write handling
     *   }
     */
    bool handleError(int fd, short revents);
    
    /**
     * ========================================================================
     * getConnectionCount() - Returns number of active connections
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns the size of the _connections map, which is the number of
     * currently active client connections.
     * 
     * WHY THIS IS USEFUL:
     * - Debugging: Log connection count periodically
     * - Monitoring: Track server load
     * - Testing: Verify connections are cleaned up properly
     * 
     * RETURNS:
     * Number of active client connections
     */
    size_t getConnectionCount() const;
    
    /**
     * ========================================================================
     * populatePollFds() - Adds all client sockets to poll_fds vector
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Iterates through all connections in _connections map and adds each
     * client socket to the provided poll_fds vector with appropriate events.
     * 
     * WHY THIS IS NEEDED:
     * EventLoop::rebuildPollFds() needs to populate poll_fds with all
     * active sockets. Since _connections is private, we need a public
     * method to expose this functionality without breaking encapsulation.
     * 
     * PARAMETERS:
     * - poll_fds: Reference to vector where pollfd structures should be added
     * 
     * SIDE EFFECTS:
     * Appends pollfd structures to poll_fds vector (does not clear it first)
     * 
     * EVENTS SET:
     * - POLLIN | POLLOUT for all client sockets
     * - We always monitor both read and write readiness
     * - POLLOUT is later cleared in handleWrite() when write_buffer is empty
     * 
     * USAGE BY EVENT LOOP:
     *   _poll_fds.clear();
     *   // Add listening sockets...
     *   _connection_manager.populatePollFds(_poll_fds);
     */
    void populatePollFds(std::vector<pollfd>& poll_fds);

private:
    /**
     * ========================================================================
     * Private Member Variables
     * ========================================================================
     */
    
    /**
     * Map of all active client connections: fd -> Connection*
     * 
     * WHY std::map INSTEAD OF std::vector:
     * - O(log n) lookup by fd (vs O(n) linear search in vector)
     * - No need for index synchronization with poll_fds array
     * - Pointer stability: Connection* addresses never change (std::vector
     *   would invalidate pointers on reallocation)
     * - Clean insertion/removal semantics
     * 
     * KEY: File descriptor (int)
     * VALUE: Pointer to Connection object (heap-allocated)
     * 
     * OWNERSHIP: ConnectionManager owns all Connection pointers. They are
     * allocated with new in addConnection() and deleted in removeConnection()
     * or destructor.
     */
    std::map<int, Connection*> _connections;
};

#endif /* CONNECTION_MANAGER_HPP */
