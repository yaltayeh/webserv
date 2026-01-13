#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "EventLoop.hpp"

// Platform-specific event constants
#if defined(__linux__)
    // Linux: epoll events
    #define EVENT_READ EPOLLIN
    #define EVENT_WRITE EPOLLOUT
    #define EVENT_EDGE EPOLLET
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: kqueue filters
    #define EVENT_READ EVFILT_READ
    #define EVENT_WRITE EVFILT_WRITE
#endif

EventLoop::EventLoop(const GlobalConfig &config)
    : _config(config), _eventFd(-1) {}

EventLoop::~EventLoop()
{
    // Close event loop file descriptor
    if (_eventFd >= 0)
    {
        close(_eventFd);
        _eventFd = -1;
    }

    // Clean up listening sockets
    for (std::map<uint64_t, ListenSocket *>::iterator it = _listeningSockets.begin();
         it != _listeningSockets.end();
         ++it)
    {
        (*it).second->close();
        delete (*it).second;
    }

    // Clean up virtual servers
    for (std::vector<VirtualServer *>::iterator it = _virtualServers.begin();
         it != _virtualServers.end();
         ++it)
    {
        delete (*it);
    }
    
    _virtualServers.clear();
    _listeningSockets.clear();
}

EventLoop::EventLoop(const EventLoop &other)
    : _eventFd(-1)
{
    this->_config = other._config;
}

EventLoop &EventLoop::operator=(const EventLoop &other)
{
    if (this != &other)
    {
        this->_config = other._config;
    }
    return (*this);
}

int EventLoop::init()
{
    ListenSocket *socket = NULL;
    VirtualServer *virtualServer = NULL;

    for (size_t i = 0; i < _config.https.size(); i++)
    {
        const std::vector<ServerConfig> &servers = _config.https[i].servers;
        for (size_t j = 0; j < servers.size(); j++)
        {
            virtualServer = new VirtualServer(servers[j]);
            _virtualServers.push_back(virtualServer);

            const std::vector<ServerConfig::ListenAddr> &addrs = servers[j].listenAddresses;
            for (size_t l = 0; l < addrs.size(); l++)
            {
                ServerConfig::ListenAddr addr = addrs[l];
                if (_listeningSockets.find(addr) == _listeningSockets.end())
                {
                    socket = new ListenSocket(addr.ip, addr.port);
                    if (socket->getSocketfd() < 0)
                    {
                        delete socket;
                        return (-1);
                    }
                    _listeningSockets[(uint64_t)addr] = socket;
                }
                else
                {
                    socket = _listeningSockets[(uint64_t)addr];
                }
                socket->addVirtualServer(virtualServer);
            }
        }
    }
    return (0);
}

// ============================================================================
// Cross-Platform Event Loop API Implementation
// ============================================================================

/**
 * Add a file descriptor to the event loop (edge-triggered, non-blocking)
 */
void EventLoop::addEventFd(int fd, bool read, bool write, void* udata)
{
    if (_eventFd < 0)
        return;

#if defined(__linux__)
    // Linux: Use epoll (edge-triggered mode with EPOLLET)
    struct epoll_event ev;
    ev.events = EPOLLET;  // Edge-triggered
    ev.data.ptr = udata;
    
    if (read)
        ev.events |= EPOLLIN;
    if (write)
        ev.events |= EPOLLOUT;
    
    if (epoll_ctl(_eventFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl ADD");
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: Use kqueue (edge-triggered with EV_CLEAR flag)
    struct kevent kev[2];
    int nevents = 0;
    
    if (read)
    {
        EV_SET(&kev[nevents], fd, EVFILT_READ,
               EV_ADD | EV_ENABLE | EV_CLEAR,  // EV_CLEAR provides edge-triggered behavior
               0, 0, udata);
        nevents++;
    }
    
    if (write)
    {
        EV_SET(&kev[nevents], fd, EVFILT_WRITE,
               EV_ADD | EV_ENABLE | EV_CLEAR,  // EV_CLEAR provides edge-triggered behavior
               0, 0, udata);
        nevents++;
    }
    
    if (nevents > 0 && kevent(_eventFd, kev, nevents, NULL, 0, NULL) == -1)
    {
        perror("kevent ADD");
    }
#endif
}

/**
 * Modify event monitoring for a file descriptor
 */
void EventLoop::modEventFd(int fd, bool read, bool write, void* udata)
{
    if (_eventFd < 0)
        return;

#if defined(__linux__)
    // Linux: Use epoll (EPOLL_CTL_MOD)
    struct epoll_event ev;
    ev.events = EPOLLET;  // Edge-triggered
    ev.data.ptr = udata;
    
    if (read)
        ev.events |= EPOLLIN;
    if (write)
        ev.events |= EPOLLOUT;
    
    if (epoll_ctl(_eventFd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        perror("epoll_ctl MOD");
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: Delete old filters and add new ones
    // kqueue doesn't have a "modify" operation, so we delete and re-add
    struct kevent kev[4];
    int nevents = 0;
    
    // Delete existing filters (if any)
    EV_SET(&kev[nevents], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    nevents++;
    EV_SET(&kev[nevents], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    nevents++;
    
    // Add new filters as requested
    if (read)
    {
        EV_SET(&kev[nevents], fd, EVFILT_READ,
               EV_ADD | EV_ENABLE | EV_CLEAR,
               0, 0, udata);
        nevents++;
    }
    
    if (write)
    {
        EV_SET(&kev[nevents], fd, EVFILT_WRITE,
               EV_ADD | EV_ENABLE | EV_CLEAR,
               0, 0, udata);
        nevents++;
    }
    
    // Note: kevent() may return error for DELETE if filter wasn't present,
    // but we can safely ignore those errors
    kevent(_eventFd, kev, nevents, NULL, 0, NULL);
#endif
}

/**
 * Remove a file descriptor from the event loop
 */
void EventLoop::delEventFd(int fd)
{
    if (_eventFd < 0)
        return;

#if defined(__linux__)
    // Linux: Use epoll (EPOLL_CTL_DEL)
    // In kernel 2.6.9+, the event argument is ignored and can be NULL
    if (epoll_ctl(_eventFd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        perror("epoll_ctl DEL");
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: Delete both read and write filters
    struct kevent kev[2];
    
    EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    
    // Note: May return error if filter wasn't present, which is fine
    kevent(_eventFd, kev, 2, NULL, 0, NULL);
#endif
}

/**
 * Wait for events with timeout (cross-platform)
 * @return Number of events triggered, -1 on error
 */
int EventLoop::waitEvents(std::vector<Event>& events, int timeout_ms)
{
    if (_eventFd < 0)
        return -1;

    events.clear();

#if defined(__linux__)
    // Linux: Use epoll_wait
    struct epoll_event epoll_events[32];
    int nfds = epoll_wait(_eventFd, epoll_events, 32, timeout_ms);
    
    if (nfds == -1)
    {
        if (errno != EINTR)
            perror("epoll_wait");
        return -1;
    }
    
    // Convert epoll events to Event structs
    for (int i = 0; i < nfds; i++)
    {
        Event ev;
        ev.udata = epoll_events[i].data.ptr;
        ev.read = (epoll_events[i].events & EPOLLIN) != 0;
        ev.write = (epoll_events[i].events & EPOLLOUT) != 0;
        ev.eof = (epoll_events[i].events & (EPOLLHUP | EPOLLRDHUP)) != 0;
        events.push_back(ev);
    }
    
    return nfds;

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: Use kevent
    struct kevent kqueue_events[32];
    struct timespec timeout;
    struct timespec* timeout_ptr = NULL;
    
    if (timeout_ms >= 0)
    {
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_nsec = (timeout_ms % 1000) * 1000000;
        timeout_ptr = &timeout;
    }
    
    int nfds = kevent(_eventFd, NULL, 0, kqueue_events, 32, timeout_ptr);
    
    if (nfds == -1)
    {
        if (errno != EINTR)
            perror("kevent");
        return -1;
    }
    
    // Convert kqueue events to Event structs
    // Note: kqueue may return multiple events for the same fd (read + write)
    for (int i = 0; i < nfds; i++)
    {
        Event ev;
        ev.udata = kqueue_events[i].udata;
        ev.read = (kqueue_events[i].filter == EVFILT_READ);
        ev.write = (kqueue_events[i].filter == EVFILT_WRITE);
        ev.eof = (kqueue_events[i].flags & EV_EOF) != 0;
        events.push_back(ev);
    }
    
    return nfds;
#endif
}

// ============================================================================
// Legacy Internal Methods (for backwards compatibility)
// ============================================================================

// ============================================================================
// Legacy Internal Methods (for backwards compatibility)
// ============================================================================

int EventLoop::addEvent(int fd, int event, void *udata)
{
    if (_eventFd < 0)
        return (-1);

#if defined(__linux__)
    // Linux: epoll - add event with edge-triggered mode
    struct epoll_event ev;
    ev.events = event | EPOLLET;
    ev.data.ptr = udata;
    if (epoll_ctl(_eventFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl ADD");
        return -1;
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: kqueue - add event with edge-triggered mode (EV_CLEAR)
    struct kevent kev;
    EV_SET(&kev, fd, event,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           0, 0, udata);
    if (kevent(_eventFd, &kev, 1, NULL, 0, NULL) == -1)
        perror("kevent ADD");
#endif

    return 0;
}

int EventLoop::deleteEvent(int fd, int event)
{
    if (_eventFd < 0)
        return (-1);

#if defined(__linux__)
    // Linux: epoll - delete fd from monitoring
    (void)event;  // Unused on Linux
    if (epoll_ctl(_eventFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        perror("epoll_ctl DEL");
        
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: kqueue - delete specific filter
    struct kevent kev;
    EV_SET(&kev, fd, event, EV_DELETE, 0, 0, NULL);
    if (kevent(_eventFd, &kev, 1, NULL, 0, NULL) == -1)
        perror("kevent DEL");
#endif

    return 0;
}

// ============================================================================
// Main Event Loop (run method)
// ============================================================================

int EventLoop::run()
{
#if defined(__linux__)
    // Linux: Create epoll instance
    _eventFd = epoll_create1(0);
    if (_eventFd < 0)
    {
        perror("epoll_create1");
        return (-1);
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: Create kqueue instance
    _eventFd = kqueue();
    if (_eventFd < 0)
    {
        perror("kqueue");
        return (-1);
    }
#endif

    // Add listen sockets to event loop
    for (std::map<uint64_t, ListenSocket *>::iterator it = _listeningSockets.begin();
         it != _listeningSockets.end();
         ++it)
    {
        addEvent((*it).second->getSocketfd(),
                 EVENT_READ,
                 (*it).second);
    }

#if defined(__linux__)
    // Linux: epoll event array
    struct epoll_event events[32];
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // macOS/BSD: kqueue event array
    struct kevent events[32];
#endif

    while (1)
    {
#if defined(__linux__)
        // Linux: Wait for epoll events (infinite timeout)
        int n = epoll_wait(_eventFd, events, 32, -1);
        
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            return -1;
        }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
        // macOS/BSD: Wait for kqueue events (infinite timeout)
        int n = kevent(_eventFd, NULL, 0, events, 32, NULL);
        
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            perror("kevent");
            return -1;
        }
#endif

        for (int i = 0; i < n; i++)
        {
#if defined(__linux__)
            // Linux: Extract user data from epoll event
            Socket *socket = (Socket *)events[i].data.ptr;
            bool is_eof = (events[i].events & (EPOLLHUP | EPOLLRDHUP)) != 0;
            bool is_read = (events[i].events & EPOLLIN) != 0;
            bool is_write = (events[i].events & EPOLLOUT) != 0;

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
            // macOS/BSD: Extract user data from kqueue event
            Socket *socket = (Socket *)events[i].udata;
            bool is_eof = (events[i].flags & EV_EOF) != 0;
            bool is_read = (events[i].filter == EVFILT_READ);
            bool is_write = (events[i].filter == EVFILT_WRITE);
#endif

            // Handle listen socket events (incoming connections)
            if (socket && socket->getType() == LISTEN_ID)
            {
                ListenSocket *listen = dynamic_cast<ListenSocket *>(socket);
                
                // Accept all pending connections (edge-triggered mode)
                while (1)
                {
                    ClientSocket *clientSocket = listen->accept();
                    if (!clientSocket)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;  // No more connections to accept
                        perror("accept");
                        break;
                    }
                    
                    // Add client socket to event loop (monitor for reads)
#if defined(__linux__)
                    addEvent(clientSocket->getSocketfd(), EVENT_READ, clientSocket);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
                    addEvent(clientSocket->getSocketfd(), EVFILT_READ, clientSocket);
#endif
                    std::cout << "add client " << clientSocket->getHost() << std::endl;
                }
                continue;
            }
            
            // Handle client socket events
            else if (socket && socket->getType() == CLIENT_ID)
            {
                ClientSocket *client = dynamic_cast<ClientSocket *>(socket);
                
                // Client disconnected (EOF/hangup)
                if (is_eof)
                {
                    std::cout << "client [" << client->getHost() << "]: disconnected" << std::endl;
#if defined(__linux__)
                    deleteEvent(client->getSocketfd(), EVENT_READ);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
                    deleteEvent(client->getSocketfd(), EVFILT_READ);
#endif
                    delete client;
                    continue;
                }
                
                // Handle read event
                if (is_read)
                {
                    if (client->handleRead() == false)
                    {
#if defined(__linux__)
                        deleteEvent(client->getSocketfd(), EVENT_READ);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
                        deleteEvent(client->getSocketfd(), EVFILT_READ);
#endif
                        delete client;
                        continue;
                    }
                    
                    // If response is ready, switch to monitoring for write
                    if (client->isComplateResponse())
                    {
#if defined(__linux__)
                        deleteEvent(client->getSocketfd(), EVENT_READ);
                        addEvent(client->getSocketfd(), EVENT_WRITE, client);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
                        deleteEvent(client->getSocketfd(), EVFILT_READ);
                        addEvent(client->getSocketfd(), EVFILT_WRITE, client);
#endif
                    }
                }
                
                // Handle write event
                else if (is_write)
                {
                    client->handleWrite();
                    
                    // If response fully sent, clean up client
                    if (client->isSendResponse())
                    {
#if defined(__linux__)
                        deleteEvent(client->getSocketfd(), EVENT_WRITE);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
                        deleteEvent(client->getSocketfd(), EVFILT_WRITE);
#endif
                        delete client;
                    }
                }
            }
        }
    }
    return (0);
}
