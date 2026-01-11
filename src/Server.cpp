/**
 * ============================================================================
 * Server.cpp - HTTP Server Instance Implementation
 * ============================================================================
 * 
 * This file implements the Server class methods. Each Server instance
 * represents one listening endpoint (host:port) with its own configuration.
 * 
 * ============================================================================
 */

#include "../include/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

/**
 * ============================================================================
 * Static Helper Functions (Internal Implementation Details)
 * ============================================================================
 */

/**
 * Sets a file descriptor to non-blocking mode
 * 
 * WHAT THIS DOES:
 * Uses fcntl() to add the O_NONBLOCK flag to the socket, which makes
 * all I/O operations on this socket non-blocking (read/write/accept
 * return immediately instead of waiting).
 * 
 * WHY THIS IS CRITICAL:
 * The 42 subject requires all I/O to go through poll(). Non-blocking
 * sockets ensure we never freeze waiting for I/O - we only read/write
 * when poll() tells us it's safe.
 * 
 * PARAMETERS:
 * - fd: File descriptor to make non-blocking
 * 
 * RETURNS:
 * - 0 on success
 * - -1 on failure
 */
static int setNonBlocking(int fd)
{
    /**
     * Get current flags on the file descriptor
     */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    
    /**
     * Add O_NONBLOCK flag and set it back
     */
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }
    
    return 0;
}

/**
 * ============================================================================
 * Server Class Method Implementations
 * ============================================================================
 */

/**
 * Constructor - Creates uninitialized Server with given configuration
 * 
 * PARAMETERS:
 * - config: ServerConfig containing host, port, routes, error pages, etc.
 */
Server::Server(const ServerConfig& config)
    : _config(config), _listen_fd(-1) {}

Server::~Server()
{
    if (_listen_fd >= 0) {
        close(_listen_fd);
        _listen_fd = -1;
    }
}

/**
 * Copy Constructor - Creates copy of another Server
 * 
 * Copies the configuration from 'other' but does NOT duplicate the
 * listening socket (that would be dangerous - two objects owning the
 * same fd). New Server starts uninitialized.
 * 
 * NOTE: After copy construction, you must call initialize() to create
 * a new listening socket.
 */
Server::Server(const Server& other)
    : _config(other._config), _listen_fd(-1) {}

/**
 * Assignment Operator - Assigns one Server to another
 * 
 * Copies configuration from 'other'. If this Server already has a
 * listening socket, closes it first. New Server is left uninitialized.
 * 
 * NOTE: _listen_fd remains -1 (uninitialized)
 * Caller must call initialize() to create new socket
 */
Server& Server::operator=(const Server& other)
{
    if (this == &other) {
        return *this;
    }
    
    // Close existing listening socket if any
    if (_listen_fd >= 0) {
        close(_listen_fd);
        _listen_fd = -1;
    }
    
    _config = other._config;
    

    return *this;
}

/**
 * initialize() - Creates and configures the listening socket
 * 
 * WHAT THIS DOES:
 * Performs complete socket setup:
 * 1. Create TCP socket
 * 2. Set SO_REUSEADDR option (allows immediate rebind)
 * 3. Bind to configured host:port
 * 4. Start listening (backlog 128)
 * 5. Set non-blocking mode
 * 
 * RETURNS:
 * - 0 on success
 * - -1 on failure (prints error to stderr)
 */
int Server::initialize()
{
    /**
     * ========================================================================
     * STEP 1: Create TCP socket (IPv4, stream-based)
     * ========================================================================
     * 
     * socket() parameters:
     * - AF_INET: IPv4 protocol
     * - SOCK_STREAM: TCP (reliable, connection-oriented)
     * - 0: Let system choose protocol (TCP for SOCK_STREAM)
     */
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0) {
        std::cerr << "Error: Failed to create socket for port "
                  << _config.port << std::endl;
        return -1;
    }
    
    /**
     * ========================================================================
     * STEP 2: Set SO_REUSEADDR socket option
     * ========================================================================
     * 
     * WHY THIS IS ESSENTIAL:
     * When a server stops, the OS keeps the port in TIME_WAIT state for
     * ~60 seconds. Without SO_REUSEADDR, restarting the server within that
     * window fails with "Address already in use". This option allows
     * immediate rebinding, which is critical for development.
     * 
     * setsockopt() parameters:
     * - _listen_fd: Socket to configure
     * - SOL_SOCKET: Set option at socket API level (not TCP/IP level)
     * - SO_REUSEADDR: Allow reuse of local addresses
     * - &opt: Pointer to option value (1 = enable)
     * - sizeof(opt): Size of option value
     */
    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: Failed to set SO_REUSEADDR on port "
                  << _config.port << std::endl;
        close(_listen_fd);
        _listen_fd = -1;
        return -1;
    }
    
    /**
     * ========================================================================
     * STEP 3: Bind socket to address (IP + port)
     * ========================================================================
     * 
     * WHAT bind() DOES:
     * Associates the socket with a specific network address and port.
     * After binding, the socket can receive connections on that address.
     * 
     * sockaddr_in structure:
     * - sin_family: Address family (AF_INET = IPv4)
     * - sin_port: Port number in network byte order (htons() converts)
     * - sin_addr.s_addr: IP address (INADDR_ANY = 0.0.0.0 = all interfaces)
     * 
     * WHY INADDR_ANY:
     * Binds to all available network interfaces. This means the server
     * accepts connections on localhost (127.0.0.1), LAN IP (192.168.x.x),
     * and public IP if available. For development, this is convenient.
     * 
     * FUTURE ENHANCEMENT:
     * Use _config.host instead of INADDR_ANY to support binding to
     * specific interfaces. This requires inet_pton() to convert string
     * IP to binary format.
     */
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_config.port);
    addr.sin_addr.s_addr = INADDR_ANY;  /** TODO: Use _config.host */
    
    if (bind(_listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: Failed to bind socket to port "
                  << _config.port << std::endl;
        close(_listen_fd);
        _listen_fd = -1;
        return -1;
    }
    
    /**
     * ========================================================================
     * STEP 4: Start listening for connections
     * ========================================================================
     * 
     * WHAT listen() DOES:
     * Marks the socket as passive (ready to accept connections). Creates
     * a queue for pending connections with specified backlog size.
     * 
     * BACKLOG PARAMETER (128):
     * Maximum number of pending connections in the queue. If queue is
     * full, new connection attempts are rejected. 128 is a standard value
     * that works well for most servers.
     * 
     * AFTER listen():
     * - Socket is ready to accept() connections
     * - Kernel starts accepting SYN packets (TCP handshake)
     * - Completed connections wait in queue until accept() is called
     */
    if (listen(_listen_fd, 128) < 0) {
        std::cerr << "Error: Failed to listen on port "
                  << _config.port << std::endl;
        close(_listen_fd);
        _listen_fd = -1;
        return -1;
    }
    
    /**
     * ========================================================================
     * STEP 5: Set socket to non-blocking mode
     * ========================================================================
     * 
     * WHY THIS IS MANDATORY:
     * The 42 subject requires non-blocking I/O with poll(). This ensures:
     * - accept() never blocks (returns -1 with EAGAIN if no connections)
     * - read() never blocks (returns -1 with EAGAIN if no data)
     * - write() never blocks (returns -1 with EAGAIN if buffer full)
     * 
     * Combined with poll(), this gives us full control over I/O timing.
     */
    if (setNonBlocking(_listen_fd) < 0) {
        std::cerr << "Error: Failed to set non-blocking mode on port "
                  << _config.port << std::endl;
        close(_listen_fd);
        _listen_fd = -1;
        return -1;
    }
    
    /**
     * ========================================================================
     * SUCCESS: Listening socket is ready
     * ========================================================================
     */
    std::cout << "Server listening on "
              << _config.host << ":" << _config.port
              << " (fd " << _listen_fd << ")" << std::endl;
    
    return 0;
}

/**
 * acceptConnection() - Accepts a new client connection
 * 
 * WHAT THIS DOES:
 * Calls accept() on the listening socket to retrieve a pending connection
 * from the queue. The new client socket is set to non-blocking mode.
 * 
 * RETURNS:
 * - Client socket fd (>= 0) on success
 * - -1 on failure or no pending connections (EAGAIN/EWOULDBLOCK)
 * 
 * IMPORTANT: Non-blocking behavior
 * If no connections are pending, accept() returns -1 immediately with
 * errno set to EAGAIN or EWOULDBLOCK. This is NOT an error - it's
 * expected behavior. Only call acceptConnection() when poll() indicates
 * POLLIN on the listening socket.
 */
int Server::acceptConnection()
{
    /**
     * accept() parameters:
     * - _listen_fd: Listening socket with pending connections
     * - &client_addr: Pointer to structure that receives client's address
     * - &addr_len: Size of address structure (modified by accept())
     * 
     * accept() returns:
     * - New socket fd for this specific client (>= 0)
     * - -1 on error or no connections pending (check errno)
     * 
     * WHAT HAPPENS:
     * - Removes one connection from the listening socket's queue
     * - Creates a brand NEW socket for this client
     * - Returns the new socket's file descriptor
     * - Listening socket remains open and keeps listening
     */
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_fd = accept(_listen_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        /**
         * Note: errno is not checked per 42 subject rules, but accept()
         * returning -1 in non-blocking mode usually means EAGAIN (no
         * connections pending). This is normal and not a real error.
         */
        return -1;
    }
    
    /**
     * CRITICAL: Set accepted socket to non-blocking mode
     * 
     * The new socket inherits settings from the listening socket, but
     * O_NONBLOCK is NOT inherited. We must explicitly set it.
     */
    if (setNonBlocking(client_fd) < 0) {
        std::cerr << "Error: Failed to set client socket non-blocking"
                  << std::endl;
        close(client_fd);
        return -1;
    }
    
    /**
     * Log successful connection (useful for debugging)
     */
    std::cout << "Server on port " << _config.port
              << " accepted connection (fd " << client_fd << ")"
              << std::endl;
    
    return client_fd;
}

/**
 * getListenFd() - Returns the listening socket file descriptor
 * 
 * RETURNS:
 * - Listening socket fd (>= 0) if initialized
 * - -1 if not initialized
 */
int Server::getListenFd() const
{
    return _listen_fd;
}

/**
 * getConfig() - Returns reference to server configuration
 * 
 * RETURNS:
 * Const reference to this Server's ServerConfig object
 * 
 * WHY CONST REFERENCE:
 * Avoids copying large config structure while preventing modification
 */
const ServerConfig& Server::getConfig() const
{
    return _config;
}
