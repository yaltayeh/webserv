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
 */

#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include "Server.hpp"
#include "ConnectionManager.hpp"
#include <vector>
#include <map>
#include <poll.h>

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop& other);
    EventLoop& operator=(const EventLoop& other);
    
    void addServer(Server* server);
    void run();
    size_t getServerCount() const;

private:
   
    void rebuildPollFds();
    
    std::vector<Server*> _servers;
    std::map<int, Server*> _listening_sockets;
    std::vector<pollfd> _poll_fds;
    ConnectionManager _connection_manager;
};

#endif
