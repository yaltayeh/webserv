#pragma once

class EventLoop;

/**
 * Platform-specific event constants
 *
 * This code compiles differently on Linux vs macOS/BSD:
 * - Linux uses epoll (efficient I/O event notification)
 * - macOS/BSD uses kqueue (BSD kernel event notification)
 *
 * Both provide edge-triggered, non-blocking I/O.
 */
#if defined(__linux__)
// Linux: epoll events
#define EVENT_READ EPOLLIN	 // Monitor for read readiness
#define EVENT_WRITE EPOLLOUT // Monitor for write readiness
#define EVENT_EDGE EPOLLET	 // Edge-triggered mode (notify once per state change)
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// macOS/BSD: kqueue filters
#define EVENT_READ EVFILT_READ	 // Monitor for read readiness
#define EVENT_WRITE EVFILT_WRITE // Monitor for write readiness
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
	bool eof;         // Connection closed/error detected
	bool readable;    // Socket has data available to read
	bool writable;    // Socket is ready to accept writes

	// --- Event Registration State (managed by EventLoop) ---
	bool monitoringRead;   // Currently monitoring for read events
	bool monitoringWrite;  // Currently monitoring for write events

	IOEventHandler();
	virtual ~IOEventHandler() = 0;
	
	// Handle the triggered I/O event (polymorphic dispatcher)
	virtual int handleEvent(EventLoop *el) = 0;
};