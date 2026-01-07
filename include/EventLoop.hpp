/**
 * ============================================================================
 * EventLoop.hpp - Main Event Loop for HTTP Server
 * ============================================================================
 * 
 * WHAT THIS CLASS DOES:
 * The EventLoop is the heart of the non-blocking HTTP server. It:
 * - Owns all Server instances (multiple servers on different ports)
 * - Owns the ConnectionManager (all client connections)
 * - Maintains the unified poll_fds array (single poll() call for ALL I/O)
 * - Dispatches events to appropriate handlers (Server or ConnectionManager)
 * - Runs the infinite loop that processes network events
 * 
 * WHY THIS DESIGN:
 * The 42 subject requires "only 1 poll() (or equivalent) for all the I/O
 * operations between the clients and the server (listen included)". This
 * means all listening sockets (multiple servers) and all client sockets must
 * be monitored by ONE poll() call. EventLoop is the orchestrator.
 * 
 * ARCHITECTURAL PATTERN:
 * EventLoop implements the Reactor pattern:
 * 1. Register: Servers register their listening sockets via addServer()
 * 2. Wait: poll() blocks until activity occurs on any registered socket
 * 3. Dispatch: EventLoop examines each ready socket and calls the appropriate
 *    handler (Server::acceptConnection or ConnectionManager::handleRead/Write)
 * 4. Repeat: Loop continues indefinitely
 * 
 * KEY DESIGN DECISIONS:
 * 
 * 1. OWNERSHIP MODEL:
 *    - EventLoop owns Server* pointers (heap-allocated, deleted in destructor)
 *    - EventLoop owns ConnectionManager (composed, stack-allocated member)
 *    - ConnectionManager owns Connection* pointers
 * 
 * 2. FD-TO-HANDLER MAPPING:
 *    - _listening_sockets map: fd -> Server* (for listening sockets)
 *    - ConnectionManager has internal map: fd -> Connection* (for clients)
 *    - When poll() detects activity on fd, check _listening_sockets first,
 *      then check ConnectionManager::hasConnection()
 * 
 * 3. poll_fds REBUILDING STRATEGY:
 *    We rebuild the entire poll_fds array before each poll() call. This is
 *    simpler and less error-prone than incremental updates:
 *    - Clear poll_fds
 *    - Add all listening sockets (iterate _listening_sockets)
 *    - Add all client sockets (iterate ConnectionManager's map)
 *    
 *    PERFORMANCE CONSIDERATION:
 *    Rebuilding seems expensive, but it's actually O(n) where n is total
 *    number of sockets. Modern CPUs handle this easily for hundreds of
 *    connections. The alternative (maintaining indices) is bug-prone and
 *    harder to debug.
 * 
 * 4. NON-BLOCKING EVERYTHING:
 *    All sockets (listening and client) are set to O_NONBLOCK mode. This
 *    ensures read(), write(), and accept() never block, which is required
 *    by the 42 subject (cannot check errno to adjust behavior).
 * 
 * INTEGRATION FLOW:
 * 
 * main() creates EventLoop:
 *   EventLoop eventLoop;
 * 
 * main() creates Server instances and adds them:
 *   Server* server1 = new Server(config1);
 *   server1->initialize();
 *   eventLoop.addServer(server1);
 * 
 * main() starts the loop:
 *   eventLoop.run();  // Never returns (infinite loop)
 * 
 * Inside run():
 *   while (true) {
 *       rebuildPollFds();  // Sync poll_fds with current sockets
 *       int ready = poll(poll_fds, ...);
 *       for (each ready socket) {
 *           if (fd is listening socket) {
 *               Server->acceptConnection();
 *               ConnectionManager->addConnection();
 *           } else if (fd is client socket) {
 *               if (revents & POLLIN) ConnectionManager->handleRead();
 *               if (revents & POLLOUT) ConnectionManager->handleWrite();
 *           }
 *       }
 *   }
 * 
 * ============================================================================
 */

#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include "Server.hpp"
#include "ConnectionManager.hpp"
#include <vector>
#include <map>
#include <poll.h>

/**
 * ============================================================================
 * EventLoop Class - Main Event Loop and Orchestrator
 * ============================================================================
 */
class EventLoop
{
public:
    /**
     * ========================================================================
     * Constructor - Creates empty EventLoop
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Initializes empty vectors/maps. No servers exist until addServer()
     * is called. ConnectionManager is default-constructed.
     */
    EventLoop();
    
    /**
     * ========================================================================
     * Destructor - Cleans up all resources
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Deletes all Server* pointers (closes listening sockets)
     * 2. ConnectionManager destructor runs automatically (closes client sockets)
     * 
     * WHY THIS IS IMPORTANT:
     * RAII principle: EventLoop owns all servers, so it must delete them.
     * Failure to delete would leak memory and leave sockets open.
     * 
     * NOTE: Order matters! Servers must be deleted before ConnectionManager
     * destructor runs, because Connections hold Server* pointers. However,
     * C++ guarantees members are destroyed in reverse order of declaration,
     * so _connection_manager (declared last) is destroyed last. This means
     * Connections are destroyed while Servers still exist, so Server*
     * pointers remain valid. Perfect!
     */
    ~EventLoop();
    
    /**
     * ========================================================================
     * Copy Constructor - DISABLED (private)
     * ========================================================================
     * 
     * WHY COPYING IS DISABLED:
     * EventLoop manages system resources (file descriptors) and dynamic
     * memory (Server pointers). Copying would create nightmarish ownership
     * issues. EventLoop should never be copied - only one instance should
     * exist per process.
     */
    EventLoop(const EventLoop& other);
    
    /**
     * ========================================================================
     * Assignment Operator - DISABLED (private)
     * ========================================================================
     * 
     * Same rationale as copy constructor.
     */
    EventLoop& operator=(const EventLoop& other);
    
    /**
     * ========================================================================
     * addServer() - Registers a new server instance
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Takes ownership of the Server* pointer
     * 2. Extracts the listening socket fd
     * 3. Stores in _listening_sockets map: fd -> Server*
     * 4. Stores in _servers vector for iteration/cleanup
     * 
     * WHY OWNERSHIP TRANSFER:
     * The caller creates the Server with new, but EventLoop takes ownership
     * by storing the pointer. EventLoop will delete it in the destructor.
     * This is similar to std::unique_ptr semantics (but we use raw pointers
     * for C++98 compatibility).
     * 
     * PARAMETERS:
     * - server: Pointer to an initialized Server (must have called initialize())
     * 
     * PRECONDITIONS:
     * - server must not be NULL
     * - server->initialize() must have succeeded (listen_fd >= 0)
     * - server's listening fd must not conflict with existing servers
     * 
     * EXAMPLE USAGE:
     *   ServerConfig config;
     *   config.host = "0.0.0.0";
     *   config.port = 8080;
     *   
     *   Server* server = new Server(config);
     *   if (server->initialize() == -1) {
     *       delete server;
     *       // Handle error
     *   }
     *   
     *   eventLoop.addServer(server);  // EventLoop now owns server
     */
    void addServer(Server* server);
    
    /**
     * ========================================================================
     * run() - Starts the main event loop (NEVER RETURNS)
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Enters an infinite loop that:
     * 1. Rebuilds poll_fds array from all sockets
     * 2. Calls poll() to wait for I/O activity
     * 3. Iterates through ready sockets
     * 4. Dispatches to appropriate handlers
     * 5. Repeats forever
     * 
     * WHY THIS NEVER RETURNS:
     * Web servers run continuously until explicitly killed (Ctrl+C, SIGTERM).
     * The infinite loop is intentional. To stop the server, send a signal
     * (future enhancement: signal handler to set a "running" flag).
     * 
     * EVENT DISPATCH LOGIC:
     * For each socket with activity (revents != 0):
     *   - If fd is in _listening_sockets:
     *       * New connection pending
     *       * Call server->acceptConnection()
     *       * Call _connection_manager.addConnection(client_fd, server)
     *   - Else if _connection_manager.hasConnection(fd):
     *       * Client socket activity
     *       * If revents & (POLLERR | POLLHUP):
     *           - Call _connection_manager.handleError()
     *           - If error, call removeConnection() and continue
     *       * If revents & POLLIN:
     *           - Call _connection_manager.handleRead()
     *           - If error, call removeConnection() and continue
     *       * If revents & POLLOUT:
     *           - Call _connection_manager.handleWrite()
     *           - If error, call removeConnection()
     * 
     * ERROR HANDLING:
     * If poll() returns -1 (system error), we print error and continue.
     * This is debatable - should we exit instead? For now, we try to be
     * resilient and keep running. A production server might want different
     * behavior (log and exit gracefully).
     * 
     * CPU OPTIMIZATION:
     * We use poll() with timeout -1 (infinite). This means the process
     * sleeps (doesn't consume CPU) until I/O activity occurs. This is far
     * more efficient than busy-waiting or short timeouts.
     * 
     * EXAMPLE USAGE:
     *   EventLoop eventLoop;
     *   // ... add servers ...
     *   eventLoop.run();  // Blocks forever, handling requests
     */
    void run();
    
    /**
     * ========================================================================
     * getServerCount() - Returns number of registered servers
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * Returns the size of the _servers vector, which is the number of
     * listening servers in this EventLoop.
     * 
     * WHY THIS IS USEFUL:
     * - Debugging: Verify correct number of servers registered
     * - Testing: Assert expected server count
     * - Logging: Report server count at startup
     * 
     * RETURNS:
     * Number of Server instances owned by this EventLoop
     */
    size_t getServerCount() const;

private:
    /**
     * ========================================================================
     * rebuildPollFds() - Rebuilds poll_fds array from current sockets
     * ========================================================================
     * 
     * WHAT THIS DOES:
     * 1. Clears the poll_fds vector
     * 2. Iterates _listening_sockets map, adds each listening socket with POLLIN
     * 3. Iterates ConnectionManager's connections, adds each client socket
     *    with POLLIN | POLLOUT (we always monitor both read and write readiness)
     * 
     * WHY REBUILD EVERY ITERATION:
     * Simplicity and correctness. The alternative is maintaining indices:
     *   poll_fds[0] = listening socket 1
     *   poll_fds[1] = listening socket 2
     *   poll_fds[2] = client socket 1
     *   ...
     * When a client disconnects, we'd need to shift indices or leave gaps.
     * Rebuilding avoids this complexity entirely.
     * 
     * PERFORMANCE:
     * O(N + M) where N = number of servers, M = number of clients.
     * For 100 servers + 1000 clients, this is ~1100 operations per poll()
     * iteration. Modern CPUs execute this in microseconds. The poll() call
     * itself takes far longer (waiting for I/O).
     * 
     * CALLED BY:
     * run() calls this before every poll() call to ensure poll_fds is
     * synchronized with current socket state.
     * 
     * NOTE: This is a private helper method, not called by external code.
     */
    void rebuildPollFds();
    
    /**
     * ========================================================================
     * Private Member Variables
     * ========================================================================
     * 
     * CRITICAL: Member declaration order matters for destruction!
     * 
     * C++ destroys members in REVERSE order of declaration. We need
     * _connection_manager destroyed BEFORE _servers because Connections
     * hold Server* pointers that must remain valid during destruction.
     * 
     * Declaration order: _servers, _listening_sockets, _poll_fds, _connection_manager
     * Destruction order: ~_connection_manager, ~_poll_fds, ~_listening_sockets, ~_servers
     * 
     * Result: Connections destroyed while Server* pointers still valid ✓
     */
    
    /**
     * Vector of all Server instances owned by this EventLoop
     * 
     * WHY std::vector<Server*>:
     * - Need to iterate all servers to rebuild poll_fds
     * - Need to delete all servers in destructor
     * - Pointers used instead of objects to avoid slicing and allow
     *   polymorphism (future: derived Server classes)
     * 
     * OWNERSHIP: EventLoop owns these pointers. They are allocated with
     * new by the caller of addServer(), and deleted in the destructor.
     * 
     * DESTRUCTION: Destroyed LAST (declared first) - ensures Server objects
     * exist while Connections are being destroyed.
     */
    std::vector<Server*> _servers;
    
    /**
     * Map of listening socket fds to Server instances
     * 
     * WHY std::map<int, Server*>:
     * - Fast O(log n) lookup during event dispatch
     * - Key is fd (from poll_fds[i].fd)
     * - Value is pointer to the Server that owns that listening socket
     * 
     * USAGE:
     * When poll() reports activity on fd:
     *   if (_listening_sockets.count(fd)) {
     *       Server* server = _listening_sockets[fd];
     *       int client_fd = server->acceptConnection();
     *       _connection_manager.addConnection(client_fd, server);
     *   }
     * 
     * SYNCHRONIZATION:
     * This map is updated in addServer() when a new Server is registered.
     * It never shrinks (servers don't get removed once added). In future,
     * we might add removeServer() for dynamic config reloading.
     */
    std::map<int, Server*> _listening_sockets;
    
    /**
     * Array of pollfd structures for poll() system call
     * 
     * WHAT IS pollfd:
     * struct pollfd {
     *     int fd;        // File descriptor to monitor
     *     short events;  // Requested events (POLLIN, POLLOUT, etc.)
     *     short revents; // Returned events (set by poll())
     * };
     * 
     * WHY std::vector<pollfd>:
     * - Dynamic size (grows/shrinks with connections)
     * - Contiguous memory (poll() requires array)
     * - Easy to pass to poll(): &poll_fds[0]
     * 
     * LIFETIME:
     * Cleared and rebuilt before each poll() call by rebuildPollFds().
     * This ensures poll_fds always reflects current socket state.
     * 
     * ORDER IN ARRAY:
     * 1. All listening sockets (from _listening_sockets)
     * 2. All client sockets (from _connection_manager)
     * 
     * NOTE: Order doesn't matter for correctness (we look up fd in maps
     * to determine type), but having a consistent order helps debugging.
     */
    std::vector<pollfd> _poll_fds;
    
    /**
     * ConnectionManager instance that owns all client connections
     * 
     * WHY COMPOSITION (NOT POINTER):
     * ConnectionManager has clear lifetime semantics: it lives as long as
     * EventLoop lives. No need for dynamic allocation - stack member is
     * simpler and safer.
     * 
     * DESTRUCTION ORDER REASONING:
     * Declared LAST, so destroyed FIRST (reverse order). This is critical!
     * When ~_connection_manager runs, it destroys all Connection objects.
     * Those Connections have Server* pointers (owner_server). By declaring
     * _servers before _connection_manager, we ensure _servers is destroyed
     * AFTER _connection_manager, meaning Server* pointers remain valid
     * during Connection destruction.
     * 
     * Visual timeline:
     * 1. ~EventLoop() begins
     * 2. ~_connection_manager runs → Connections destroyed (Server* still valid)
     * 3. ~_poll_fds runs (just vector cleanup)
     * 4. ~_listening_sockets runs (just map cleanup)
     * 5. ~_servers runs → Server pointers deleted
     * 6. ~EventLoop() completes
     */
    ConnectionManager _connection_manager;
};

#endif /* EVENT_LOOP_HPP */
