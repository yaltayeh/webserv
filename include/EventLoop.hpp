#pragma once

class EventLoop;

#include <vector>
#include <map>
#include "ListenSocket.hpp"
#include "Config/GlobalConfig.hpp"
#include "IndexedQueue.hpp"
#include <ctime>
#include "ChildProcessHandler.hpp"

// Platform-specific includes
#if defined(__linux__)
#include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/event.h>
#include <sys/time.h>
#endif

/**
 * Cross-platform event loop supporting epoll (Linux) and kqueue (macOS/BSD)
 * Provides edge-triggered, non-blocking event notifications
 * Uses IndexedQueue for O(1) client activity tracking and timeout detection
 */
class EventLoop
{
private:

    GlobalConfig    _config;

    std::vector<VirtualServer *>        _virtualServers;
    std::map<uint64_t, ListenSocket *>  _listeningSockets;
    int                                 _eventFd; // epoll fd (Linux) or kqueue fd (macOS/BSD)

    // O(1) queue for client activity tracking
    // Key: socket fd, Value: ClientSocket*
    // Front = oldest (least recently active), Back = newest (most recently active)
    IndexedQueue<int, ClientSocket *>   _clientQueue;
    std::map<pid_t, ChildProcessHandler *>     _childProcesses;
    std::list<ClientSocket *>              _deleteClients;

    // Check for timed-out clients and close them
    void checkClientTimeouts();

    // Check for exited child processes
    void CheckChildProcesses();

public:

    EventLoop(const GlobalConfig &config);
    ~EventLoop();

    // Add or update client activity (moves to back if exists)
    void    markClientActive(ClientSocket *client);
    
    // Remove client from activity queue (called by ClientSocket::cleanUp)
    void    removeClient(ClientSocket *client);
    
    // Request to close and delete a client socket (EventLoop owns cleanup)
    void    clientDisconnect(ClientSocket *client);

    void    addChildProcess(ChildProcessHandler *handler);
    void    removeChildProcess(ChildProcessHandler *handler);

    int     addEvent(int fd, int event, IOEventHandler *udata);
    int     deleteEvent(int fd, int event, IOEventHandler *udata);
    void    run(); // Main event loop

};
