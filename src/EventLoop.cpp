#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include "EventLoop.hpp"

/**
 * Constructor: Initialize event loop with configuration
 * _eventFd is set to -1 (invalid) until run() creates the epoll/kqueue fd
 */
EventLoop::EventLoop(const GlobalConfig &config)
	: _config(config), _eventFd(-1)
{
	ListenSocket *socket = NULL;
	VirtualServer *virtualServer = NULL;

	// Loop through all HTTP configurations
	for (size_t i = 0; i < _config.https.size(); i++)
	{
		const std::vector<ServerConfig> &servers = _config.https[i].servers;

		// Loop through all server blocks
		for (size_t j = 0; j < servers.size(); j++)
		{
			// Create virtual server (handles routing for this server block)
			virtualServer = new VirtualServer(servers[j]);
			_virtualServers.push_back(virtualServer);

			// Loop through all listen addresses for this server
			const std::vector<ServerConfig::ListenAddr> &addrs = servers[j].listenAddresses;
			for (size_t l = 0; l < addrs.size(); l++)
			{
				ServerConfig::ListenAddr addr = addrs[l];

				// Check if we already have a socket for this host:port
				if (_listeningSockets.find(addr) == _listeningSockets.end())
				{
					// Create new listen socket (bind to host:port)
					socket = new ListenSocket(addr.ip, addr.port);
					if (socket->getSocketfd() < 0)
					{
						delete socket;
						std::ostringstream oss;
						oss << "Failed to create listen socket for " << addr.ip << ":" << addr.port;
						throw std::runtime_error(oss.str());
					}
					_listeningSockets[(uint64_t)addr] = socket;
				}
				else
				{
					// Reuse existing socket for this host:port
					socket = _listeningSockets[(uint64_t)addr];
				}

				// Associate this virtual server with the listen socket
				socket->addVirtualServer(virtualServer);
			}
		}
	}
}

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

/**
 * Add a file descriptor to the event loop for monitoring
 *
 *
 * @param fd     File descriptor to monitor (socket)
 * @param event  Type of event (EVENT_READ or EVENT_WRITE)
 * @param udata  User data pointer (EventData* to identify which socket triggered)
 *
 * Edge-triggered mode means:
 * - Notification happens once per state change
 * - Must read/write until EAGAIN to avoid missing events
 */
int EventLoop::addEvent(int fd, int event, EventData *udata)
{
	if (_eventFd < 0)
		return (-1);

	if (event == EVENT_READ && udata->isEnableReadEvent)
		return (0);
	if (event == EVENT_WRITE && udata->isEnableWriteEvent)
		return (0);

#if defined(__linux__)
	// Linux: Use epoll to add file descriptor
	struct epoll_event ev;
	ev.events = event | EPOLLET; // Combine requested event with edge-triggered flag
	ev.data.ptr = udata;		 // Store Socket* for later retrieval

	if (epoll_ctl(_eventFd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		perror("epoll_ctl ADD");
		return -1;
	}

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: Use kqueue to add file descriptor
	struct kevent kev;
	EV_SET(&kev, fd, event,
		   EV_ADD | EV_ENABLE | EV_CLEAR, // Add, enable, and clear (edge-triggered)
		   0, 0, udata);				  // Store Socket* for later retrieval

	if (kevent(_eventFd, &kev, 1, NULL, 0, NULL) == -1)
		perror("kevent ADD");
#endif

	if (event == EVENT_READ)
		udata->isEnableReadEvent = true;
	if (event == EVENT_WRITE)
		udata->isEnableWriteEvent = true;
	return 0;
}

/**
 * Remove a file descriptor from the event loop
 *
 *
 * @param fd     File descriptor to stop monitoring
 * @param event  Type of event (EVENT_READ or EVENT_WRITE)
 *
 *
 * Note: On Linux, epoll removes the entire fd (not just one event type)
 *       On macOS/BSD, kqueue removes the specific filter
 */
int EventLoop::deleteEvent(int fd, int event, EventData *udata)
{
	if (_eventFd < 0)
		return (-1);

	if (event == EVENT_READ && !(udata->isEnableReadEvent))
		return (0);
	if (event == EVENT_WRITE && !(udata->isEnableWriteEvent))
		return (0);

#if defined(__linux__)
	// Linux: epoll removes entire fd, not individual event types
	(void)event; // Suppress unused parameter warning

	if (epoll_ctl(_eventFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		perror("epoll_ctl DEL");

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: kqueue removes specific filter (READ or WRITE)
	struct kevent kev;
	EV_SET(&kev, fd, event, EV_DELETE, 0, 0, NULL);

	if (kevent(_eventFd, &kev, 1, NULL, 0, NULL) == -1)
		perror("kevent DEL");
#endif

	if (event == EVENT_READ)
		udata->isEnableReadEvent = false;
	if (event == EVENT_WRITE)
		udata->isEnableWriteEvent = false;

	return 0;
}

// ============================================================================
// Main Event Loop (run method)
// ============================================================================

/**
 * Main event loop - runs forever processing network events
 *
 *
 * Flow:
 * 1. Create event loop fd (epoll on Linux, kqueue on macOS)
 * 2. Add all listen sockets to monitor for incoming connections
 * 3. Enter infinite loop:
 *    - Wait for events (connections, data ready to read/write)
 *    - Process each event:
 *      * Listen socket → accept new connections
 *      * Client socket read → receive HTTP request
 *      * Client socket write → send HTTP response
 * 4. Use edge-triggered I/O for efficiency
 */
void EventLoop::run()
{
#if defined(__linux__)
	// Linux: Create epoll instance (event notification facility)
	_eventFd = epoll_create(1);
	if (_eventFd < 0)
		throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: Create kqueue instance (kernel event queue)
	_eventFd = kqueue();
	if (_eventFd < 0)
		throw std::runtime_error("Failed to create kqueue instance: " + std::string(strerror(errno)));
#endif

	// Set FD_CLOEXEC flag to prevent fd leaks to child processes
	int flags = fcntl(_eventFd, F_GETFD);
	if (flags == -1 || fcntl(_eventFd, F_SETFD, flags | FD_CLOEXEC) == -1)
	{
		close(_eventFd);
		throw std::runtime_error("Failed to set FD_CLOEXEC on kqueue fd: " + std::string(strerror(errno)));
	}

	// Add all listen sockets to event loop (monitor for incoming connections)
	for (std::map<uint64_t, ListenSocket *>::iterator it = _listeningSockets.begin();
		 it != _listeningSockets.end();
		 ++it)
	{
		addEvent((*it).second->getSocketfd(), EVENT_READ, (*it).second);
	}

#if defined(__linux__)
	// Linux: Array to hold triggered epoll events (max 32 events per wait)
	struct epoll_event events[1024];
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: Array to hold triggered kqueue events (max 32 events per wait)
	struct kevent events[1024];
#endif

	int maxevents = sizeof(events) / sizeof(events[0]);

	// Infinite event loop
	while (1)
	{
#if defined(__linux__)
		// Linux: Wait for events (blocks until something happens)
		// Returns number of triggered events
		int n = epoll_wait(_eventFd, events, maxevents, -1); // -1 = wait forever

		if (n == -1)
		{
			// EINTR means interrupted by signal (not a real error)
			if (errno == EINTR)
				continue; // Try again
			throw std::runtime_error("epoll_wait failed: " + std::string(strerror(errno)));
		}

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
		// macOS/BSD: Wait for events (blocks until something happens)
		// Returns number of triggered events
		int n = kevent(_eventFd, NULL, 0, events, maxevents, NULL); // NULL = wait forever

		if (n == -1)
		{
			// EINTR means interrupted by signal (not a real error)
			if (errno == EINTR)
				continue; // Try again
			throw std::runtime_error("kevent failed: " + std::string(strerror(errno)));
		}
#endif

		// while (waitpid(-1, NULL, WNOHANG) != -1)
		// 	;

		// Process each triggered event
		for (int i = 0; i < n; i++)
		{
#if defined(__linux__)
			// Linux: Extract socket and event type from epoll event
			EventData *event = (EventData *)events[i].data.ptr;
			if (!event)
				continue;

			// Check what happened on this socket
			event->isEOF = (events[i].events & (EPOLLHUP | EPOLLRDHUP)) != 0; // Connection closed
			event->isRead = (events[i].events & EPOLLIN) != 0;				  // Data available to read
			event->isWrite = (events[i].events & EPOLLOUT) != 0;			  // Socket ready for writing

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
			// macOS/BSD: Extract socket and event type from kqueue event
			EventData *event = (EventData *)events[i].udata;
			if (!event)
				continue;
			// Check what happened on this socket
			event->isEOF = (events[i].flags & EV_EOF) != 0;		 // Connection closed
			event->isRead = (events[i].filter == EVFILT_READ);	 // Data available to read
			event->isWrite = (events[i].filter == EVFILT_WRITE); // Socket ready for writing
#endif
			event->EventHandler(this);

		} // End of event processing loop
	} // End of infinite while loop
}
