#pragma once

#include <vector>
#include <map>
#include "ListenSocket.hpp"
#include "Config/GlobalConfig.hpp"

// Platform-specific includes
#if defined(__linux__)
    #include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #include <sys/event.h>
    #include <sys/time.h>
#endif

/**
 * Event structure for cross-platform event notification
 * Contains user data pointer and flags indicating read/write readiness
 */
struct Event
{
    void* udata;      // User data pointer associated with the file descriptor
    bool read;        // True if fd is ready for reading
    bool write;       // True if fd is ready for writing
    bool eof;         // True if connection closed (peer disconnected)
    
    Event() : udata(NULL), read(false), write(false), eof(false) {}
};

/**
 * Cross-platform event loop supporting epoll (Linux) and kqueue (macOS/BSD)
 * Provides edge-triggered, non-blocking event notifications
 */
class EventLoop
{
private:
    GlobalConfig _config;

    std::vector<VirtualServer *> _virtualServers;
    std::map<uint64_t, ListenSocket *> _listeningSockets;

    int _eventFd;  // epoll fd (Linux) or kqueue fd (macOS/BSD)

    // Internal helper methods for backwards compatibility
    int addEvent(int fd, int event, void *udata);
    int deleteEvent(int fd, int event);

public:
    EventLoop(const GlobalConfig &config);
    ~EventLoop();
    EventLoop(const EventLoop &other);
    EventLoop &operator=(const EventLoop &other);

    // Cross-platform event loop API
    /**
     * Add a file descriptor to the event loop
     * @param fd File descriptor to monitor
     * @param read Monitor for read events
     * @param write Monitor for write events
     * @param udata User data pointer to associate with events
     */
    void addEventFd(int fd, bool read, bool write, void* udata);
    
    /**
     * Modify event monitoring for a file descriptor
     * @param fd File descriptor to modify
     * @param read Monitor for read events
     * @param write Monitor for write events
     * @param udata User data pointer to associate with events
     */
    void modEventFd(int fd, bool read, bool write, void* udata);
    
    /**
     * Remove a file descriptor from the event loop
     * @param fd File descriptor to remove
     */
    void delEventFd(int fd);
    
    /**
     * Wait for events with timeout
     * @param events Vector to store triggered events
     * @param timeout_ms Timeout in milliseconds (-1 for infinite)
     * @return Number of events triggered, -1 on error
     */
    int waitEvents(std::vector<Event>& events, int timeout_ms);

    // Initialization and main loop
    int init();
    int run();
};
