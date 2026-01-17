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

struct EventData
{
	bool isEOF;
	bool isWrite;
	bool isRead;

	bool isEnableReadEvent;
	bool isEnableWriteEvent;

	EventData();
	virtual ~EventData() = 0;
	virtual int EventHandler(EventLoop *el) = 0;
};
