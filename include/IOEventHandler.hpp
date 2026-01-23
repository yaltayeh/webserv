#pragma once

#include <stdint.h>
class EventLoop;

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

#include <stdint.h>

// Portable flags
enum IOEventFlags {
    IO_NONE  = 0,
    IO_READ  = 1 << 0,
    IO_WRITE = 1 << 1,
    IO_ERROR = 1 << 2,
    IO_HUP   = 1 << 3
};

enum IOEventState {
    IO_EVENT_CONTINUE = 0,
    IO_EVENT_CLOSE    = 1,
};

// Convert Linux epoll events to portable flags
#if defined(__linux__)
inline uint32_t mapEpollToIOFlags(uint32_t epollEvents) {
    uint32_t flags = IO_NONE;
    if (epollEvents & EPOLLIN)  flags |= IO_READ;
    if (epollEvents & EPOLLOUT) flags |= IO_WRITE;
    if (epollEvents & EPOLLERR) flags |= IO_ERROR;
    if (epollEvents & EPOLLHUP) flags |= IO_HUP;
    if (epollEvents & EPOLLRDHUP) flags |= IO_HUP; // treat half-close as HUP
    return flags;
}
#endif

// Convert macOS kqueue events to portable flags
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
inline uint32_t mapKqueueToIOFlags(const struct kevent &kev) {
    uint32_t flags = IO_NONE;
    if (kev.filter == EVFILT_READ)  flags |= IO_READ;
    if (kev.filter == EVFILT_WRITE) flags |= IO_WRITE;
    if (kev.flags & EV_ERROR)        flags |= IO_ERROR;
    if (kev.flags & EV_EOF)          flags |= IO_HUP;
    return flags;
}
#endif




/**
 * Abstract base class for I/O event handling in the event loop.
 * 
 * Combines two responsibilities:
 * 1. Event state (which I/O events were triggered)
 * 2. Event registration tracking (which events are being monitored)
 * 
 * Subclasses: ListenSocket, ClientSocket, StreamSocket
 */
struct IOEventHandler
{
	// --- Event State (set by EventLoop before calling handleEvent) ---
	// bool eof;        // Connection closed/error detected
	// bool readable;   // Socket has data available to read
	// bool writable;   // Socket is ready to accept writes
	uint32_t events;    // Bitmask of triggered events

	// --- Event Registration State (managed by EventLoop) ---
	bool monitoringRead;   // Currently monitoring for read events
	bool monitoringWrite;  // Currently monitoring for write events

	IOEventHandler() : events(IO_NONE), monitoringRead(false), monitoringWrite(false) { }
	virtual ~IOEventHandler() { };
	
	// Handle the triggered I/O event (polymorphic dispatcher)
	virtual IOEventState handleEvent(EventLoop *el) = 0;
    // Handle cleanup when the handler is being destroyed
    virtual void handleDestroy(EventLoop *el) = 0;
};
