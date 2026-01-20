#pragma once

class EventLoop;

#include <vector>
#include <map>
#include "ListenSocket.hpp"
#include "Config/GlobalConfig.hpp"
#include <queue>

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
 */
class EventLoop
{
private:

    GlobalConfig    _config;

    std::vector<VirtualServer *>        _virtualServers;
    std::map<uint64_t, ListenSocket *>  _listeningSockets;
    int                                 _eventFd; // epoll fd (Linux) or kqueue fd (macOS/BSD)


    std::queue<ClientSocket *>   _clientQueue;

public:

    EventLoop(const GlobalConfig &config);
    ~EventLoop();

    void    addToQueue(ClientSocket *client);
    void    removeFromQueue(ClientSocket *client);

    int     addEvent(int fd, int event, IOEventHandler *udata);
    int     deleteEvent(int fd, int event, IOEventHandler *udata);
    void    run(); // Main event loop

};
