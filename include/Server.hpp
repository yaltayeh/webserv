/**
 * ============================================================================
 * Server.hpp - HTTP Server Instance Class
 * ============================================================================
 * 
 * WHAT THIS CLASS DOES:
 * Represents a single HTTP server instance that listens on a specific host:port
 * pair. Each Server owns its listening socket, configuration (routes, error
 * pages, body limits), and handles accepting new client connections.
 * 
 * WHY THIS CLASS EXISTS:
 * The 42 subject requires supporting multiple server blocks (like NGINX),
 * where each server can have different configurations. This class encapsulates
 * per-server state and behavior, making it easy to run multiple servers in
 * a single event loop with one poll() call.
 * 
 * OWNERSHIP AND LIFECYCLE:
 * - Server instances are created and owned by the EventLoop
 * - Each Server creates its own listening socket during initialize()
 * - The listening socket fd is registered in EventLoop's poll_fds array
 * - Server destructor closes the listening socket automatically
 * - Clients accepted by this Server are associated with it via Connection::owner_server
 * 
 * KEY DESIGN PATTERNS:
 * - RAII (Resource Acquisition Is Initialization): Socket is created in
 *   initialize() and closed in destructor, preventing resource leaks
 * - Orthodox Canonical Form: Copy constructor, assignment operator, destructor
 *   properly implemented for C++98 compliance
 * - Composition: Server HAS-A ServerConfig (stores configuration)
 * 
 * INTEGRATION WITH EVENT LOOP:
 * 1. EventLoop creates Server instances with different configs
 * 2. EventLoop calls server.initialize() to create listening sockets
 * 3. EventLoop stores server.getListenFd() in a map: fd -> Server*
 * 4. When poll() detects activity on listening socket, EventLoop dispatches
 *    to server.acceptConnection() to handle the new client
 * 5. Accepted connection is given to ConnectionManager with owner_server pointer
 * 
 * ============================================================================
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config/ServerConfig.hpp"
#include <string>

/**
 * Forward declarations to avoid circular dependencies
 * 
 * WHY THIS IS NEEDED:
 * Server.hpp is included by EventLoop.hpp and ConnectionManager.hpp.
 * If we #include those headers here, we create circular dependencies.
 * Forward declarations let us use pointers/references without full definitions.
 */
class EventLoop;
class ConnectionManager;

/**
 * ============================================================================
 * Server Class - Single HTTP Server Instance
 * ============================================================================
 */
class Server
{
public:
    /**
     * ========================================================================
     * Constructor - Creates uninitialized Server with given configuration
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Creates a Server object with the specified configuration. The listening
     * socket is NOT created yet - call initialize() after construction.
     * 
     * WHY TWO-PHASE INITIALIZATION:
     * Socket creation can fail (port already in use, permission denied). By
     * separating construction from initialization, we can handle errors
     * gracefully without throwing exceptions (exceptions complicate C++98).
     * 
     * PARAMETERS:
     * - config: ServerConfig object containing host, port, routes, etc.
     *           This is copied into the Server (value semantics).
     * 
     * EXAMPLE USAGE:
     *   ServerConfig cfg;
     *   cfg.host = "0.0.0.0";
     *   cfg.port = 8080;
     *   Server server(cfg);
     *   if (server.initialize() == -1) {
     *       // Handle error
     *   }
     */
    Server(const ServerConfig& config);
    
    /**
     * ========================================================================
     * Destructor - Cleans up resources (closes listening socket)
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Closes the listening socket if it's open (fd != -1). This ensures
     * no resource leaks occur when Server objects are destroyed.
     * 
     * WHY THIS IS IMPORTANT:
     * RAII principle: Resources acquired during lifetime should be released
     * automatically. The OS will close sockets when process exits, but
     * explicit closure is cleaner and allows port reuse immediately.
     * 
     * NOTE: This does NOT close client connections - those are managed by
     * ConnectionManager. Only the listening socket is closed here.
     */
    ~Server();
    
    /**
     * ========================================================================
     * Copy Constructor - Creates a copy of another Server
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Creates a new Server with the same configuration as 'other'. The
     * listening socket is NOT duplicated (that would be dangerous - two
     * objects managing the same fd). The new Server starts uninitialized.
     * 
     * WHY THIS EXISTS:
     * Orthodox Canonical Form requires copy constructor for C++98 classes.
     * However, copying Servers is generally NOT recommended - they should
     * be created once and stored in containers by pointer.
     * 
     * PARAMETERS:
     * - other: Server to copy configuration from
     * 
     * NOTE: After copy construction, you must call initialize() to create
     * a new listening socket.
     */
    Server(const Server& other);
    
    /**
     * ========================================================================
     * Assignment Operator - Assigns one Server to another
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Copies configuration from 'other' to this Server. If this Server
     * already has a listening socket open, it is closed first. The new
     * Server is left uninitialized (must call initialize() again).
     * 
     * WHY THIS EXISTS:
     * Orthodox Canonical Form requires assignment operator for C++98.
     * However, assigning Servers is generally NOT recommended.
     * 
     * PARAMETERS:
     * - other: Server to copy configuration from
     * 
     * RETURNS:
     * Reference to this Server (allows chaining: a = b = c)
     */
    Server& operator=(const Server& other);
    
    /**
     * ========================================================================
     * initialize() - Creates and configures the listening socket
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Creates a TCP socket (socket() system call)
     * 2. Sets SO_REUSEADDR option (allows immediate rebind after restart)
     * 3. Binds to the configured host:port (bind() system call)
     * 4. Starts listening for connections (listen() with backlog 128)
     * 5. Sets socket to non-blocking mode (fcntl() with O_NONBLOCK)
     * 
     * WHY EACH STEP IS IMPORTANT:
     * - SO_REUSEADDR: Without this, restarting the server within 60s fails
     *   with "Address already in use" (TIME_WAIT state)
     * - Non-blocking: Required by 42 subject - all I/O must go through poll()
     * - Backlog 128: Queue size for pending connections (standard value)
     * 
     * RETURNS:
     * - 0 on success
     * - -1 on failure (socket creation, bind, or listen failed)
     * 
     * ERROR HANDLING:
     * On failure, prints error message to stderr but does NOT exit. Caller
     * should check return value and decide whether to continue or abort.
     * 
     * EXAMPLE USAGE:
     *   Server server(config);
     *   if (server.initialize() == -1) {
     *       std::cerr << "Failed to start server on port " 
     *                 << config.port << std::endl;
     *       return 1;
     *   }
     */
    int initialize();
    
    /**
     * ========================================================================
     * acceptConnection() - Accepts a new client connection
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Calls accept() on the listening socket to accept a pending connection.
     * The new client socket is set to non-blocking mode immediately.
     * 
     * WHY THIS IS A SERVER METHOD:
     * Each Server handles accepting its own clients. This keeps the Server
     * class cohesive and makes it easy to track which clients belong to
     * which server configuration.
     * 
     * RETURNS:
     * - Client socket fd (>= 0) on success
     * - -1 on failure or if no connections are pending (EWOULDBLOCK/EAGAIN)
     * 
     * INTEGRATION WITH EVENT LOOP:
     * When EventLoop detects POLLIN on a listening socket, it looks up the
     * corresponding Server and calls this method. The returned fd is then
     * passed to ConnectionManager to create a new Connection object.
     * 
     * EXAMPLE USAGE:
     *   int client_fd = server.acceptConnection();
     *   if (client_fd != -1) {
     *       connectionManager.addConnection(client_fd, &server);
     *   }
     */
    int acceptConnection();
    
    /**
     * ========================================================================
     * getListenFd() - Returns the listening socket file descriptor
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns the file descriptor of the listening socket. This is used by
     * EventLoop to register the socket in poll_fds array and to check if
     * a particular fd is a listening socket.
     * 
     * RETURNS:
     * - Listening socket fd (>= 0) if initialized
     * - -1 if not initialized
     * 
     * USAGE BY EVENT LOOP:
     * EventLoop maintains a map: fd -> Server*
     * After server.initialize(), EventLoop does:
     *   listening_sockets[server.getListenFd()] = &server;
     * 
     * During poll() dispatch:
     *   if (listening_sockets.count(fd)) {
     *       Server* server = listening_sockets[fd];
     *       int client_fd = server->acceptConnection();
     *   }
     */
    int getListenFd() const;
    
    /**
     * ========================================================================
     * getConfig() - Returns reference to server configuration
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns a const reference to the ServerConfig object stored in this
     * Server. This allows read-only access to configuration data without
     * copying.
     * 
     * WHY CONST REFERENCE:
     * - Avoids expensive copy of entire config structure
     * - Const prevents accidental modification
     * - Reference semantics: caller gets direct access to internal data
     * 
     * RETURNS:
     * Const reference to this Server's ServerConfig
     * 
     * USAGE BY CONNECTION HANDLER:
     * When processing a client request, ConnectionManager needs to know
     * which routes, error pages, and body limits apply:
     *   const ServerConfig& cfg = connection.owner_server->getConfig();
     *   for (size_t i = 0; i < cfg.routes.size(); ++i) {
     *       if (matches(uri, cfg.routes[i].path)) { ... }
     *   }
     */
    const ServerConfig& getConfig() const;

private:
    /**
     * ========================================================================
     * Private Member Variables
     * ========================================================================
     */
    
    /**
     * Server configuration (host, port, routes, error pages, etc.)
     * 
     * This is a complete copy of the ServerConfig passed to the constructor.
     * Value semantics ensure the Server owns its configuration independently.
     */
    ServerConfig _config;
    
    /**
     * Listening socket file descriptor
     * 
     * Valid values: >= 0 (valid socket fd)
     * Special value: -1 (not initialized or closed)
     * 
     * This fd is:
     * - Created in initialize()
     * - Registered in EventLoop's poll_fds array
     * - Monitored for POLLIN events (new connections pending)
     * - Closed in destructor
     */
    int _listen_fd;
};

#endif /* SERVER_HPP */
