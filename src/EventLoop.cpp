#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include "EventLoop.hpp"
#include "IOEventHandler.hpp"
#include <sys/wait.h>
#include "libutils/include/Logger.hpp"
// #include <chrono>

/**
 * Constructor: Initialize event loop with configuration
 * _eventFd is set to -1 (invalid) until run() creates the epoll/kqueue fd
 * Initializes IndexedQueue with estimated capacity for 1024 concurrent connections
 */
EventLoop::EventLoop(const GlobalConfig &config)
	: _config(config), _eventFd(-1), _clientQueue(1024)
{
	ListenSocket *socket = NULL;
	VirtualServer *virtualServer = NULL;

	// Loop through all HTTP configurations
	const std::vector<ServerConfig> &servers = _config.http.servers;

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
 * Mark client as active by updating its position in the queue
 * If client exists: moves to back (most recent activity)
 * If new: adds to back
 * O(1) operation using IndexedQueue
 */
void EventLoop::markClientActive(ClientSocket *client)
{
	if (!client)
		return;

	// Update expiry date: current time + timeout
	client->setExpiryDate(time(NULL) + _config.http.idle_timeout);

	// O(1): Push to back (or move to back if already exists)
	// Parameters: (key, data) where key=fd, data=ClientSocket*
	_clientQueue.push(client->getSocketfd(), client);
}

/**
 * Remove client from activity queue
 * Called by ClientSocket::cleanUp()
 */
void EventLoop::removeClient(ClientSocket *client)
{
	if (!client)
		return;

	_clientQueue.remove(client->getSocketfd());
}

/**
 * Request to close a client connection (EventLoop is the owner)
 * Removes from queue, deletes events, and frees memory
 * This is the ONLY way to delete a ClientSocket
 */
void EventLoop::clientDisconnect(ClientSocket *client)
{
	if (!client)
		return;

	// Check if already marked for deletion (prevent double-disconnect)
	for (std::list<ClientSocket *>::iterator it = _deleteClients.begin();
		 it != _deleteClients.end(); ++it)
	{
		if (*it == client)
			return; // Already marked for deletion
	}

	// Remove from activity queue (O(1) average)
	if (!_clientQueue.remove(client->getSocketfd()))
		return;

	// std::cout << *client << " Marking client for deletion" << std::endl;

	// Remove from event monitoring
	if (client->getSocketfd() >= 0)
		deleteEvent(client->getSocketfd(), IO_READ | IO_WRITE, client);

	// Close the socket but don't delete yet
	client->close();
	
	LOG_DEBUG("Marking client for deletion: " << *client);
	// Add to deletion list - will be deleted at end of event loop iteration
	_deleteClients.push_back(client);
}

void EventLoop::addChildProcess(ChildProcessHandler *handler)
{
	if (!handler)
		return;
	_childProcesses[handler->getPid()] = handler;
}

void EventLoop::removeChildProcess(ChildProcessHandler *handler)
{
	if (!handler)
		return;
	_childProcesses.erase(handler->getPid());
}

/**
 * Check for timed-out clients and close them
 * Front of queue = oldest (least recently active) connection
 * O(1) per timeout check, stops when we hit a non-expired connection
 */
void EventLoop::checkClientTimeouts()
{
	time_t now = time(NULL);

	// Check oldest connections first (front of queue)
	while (!_clientQueue.isEmpty())
	{
		ClientSocket *oldest = _clientQueue.front();

		// If oldest is not expired, none of the others are either
		if (oldest->getExpiryDate() > now)
			break;

		// Timed out - pop from queue and close
		LOG_DEBUG("Timeout: closing idle connection " << *oldest);
		_clientQueue.pop(); // Remove from front

		// Request closure
		clientDisconnect(oldest);
	}
}

void EventLoop::CheckChildProcesses()
{
	pid_t pid;
	int status;

	// Check for any exited child processes
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if (WIFEXITED(status))
			LOG_DEBUG("Child process " << pid << " exited with status " << WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			LOG_DEBUG("Child process " << pid << " terminated by signal " << WTERMSIG(status));
		std::map<pid_t, ChildProcessHandler *>::iterator it = _childProcesses.find(pid);
		if (it != _childProcesses.end())
		{
			ChildProcessHandler *handler = (*it).second;
			// Remove from map first to prevent double-handling
			_childProcesses.erase(it);
			handler->handleChildExit(status);
			handler->handleDestroy(this);
			delete handler;
			// Note: handler might have already been destroyed by event loop,
			// so we don't call handleDestroy here
		}
	}
}

/**
 * Add a file descriptor to the event loop for monitoring
 *
 *
 * @param fd     File descriptor to monitor (socket)
 * @param event  Type of event (IO_READ or IO_WRITE)
 * @param udata  User data pointer (EventData* to identify which socket triggered)
 *
 * Edge-triggered mode means:
 * - Notification happens once per state change
 * - Must read/write until EAGAIN to avoid missing events
 */
int EventLoop::addEvent(int fd, int event, IOEventHandler *udata)
{
	if (_eventFd < 0)
		return (-1);

	// Check which events need to be added
	bool needAddRead = (event & IO_READ) && !udata->monitoringRead;
	bool needAddWrite = (event & IO_WRITE) && !udata->monitoringWrite;
	
	if (!needAddRead && !needAddWrite)
		return (0); // Nothing to add

#if defined(__linux__)
	// Linux: Use epoll to add file descriptor
	struct epoll_event ev;
	// Map portable IO flags to epoll flags
	ev.events = 0; // level-triggered by default
	
	// Include all currently monitored events
	if (udata->monitoringRead || needAddRead)
		ev.events |= EPOLLIN;
	if (udata->monitoringWrite || needAddWrite)
		ev.events |= EPOLLOUT;
	
	ev.data.ptr = udata; // Store Socket* for later retrieval

	// Try to add first, if it fails with EEXIST, modify instead
	int op = (udata->monitoringRead || udata->monitoringWrite) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	
	if (epoll_ctl(_eventFd, op, fd, &ev) == -1)
	{
		// If ADD failed because fd exists, try MOD
		if (op == EPOLL_CTL_ADD && errno == EEXIST)
		{
			if (epoll_ctl(_eventFd, EPOLL_CTL_MOD, fd, &ev) == -1)
			{
				perror("epoll_ctl MOD");
				return -1;
			}
		}
		else
		{
			perror("epoll_ctl ADD");
			return -1;
		}
	}

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: Use kqueue to add file descriptor
	// kqueue requires separate kevent calls for each filter (READ and WRITE)
	struct kevent kevs[2];
	int kev_count = 0;
	
	if (needAddRead)
	{
		EV_SET(&kevs[kev_count], fd, EVFILT_READ,
			   EV_ADD | EV_ENABLE, // Add, enable level-triggered
			   0, 0, udata);	   // Store Socket* for later retrieval
		kev_count++;
	}
	
	if (needAddWrite)
	{
		EV_SET(&kevs[kev_count], fd, EVFILT_WRITE,
			   EV_ADD | EV_ENABLE, // Add, enable level-triggered
			   0, 0, udata);	   // Store Socket* for later retrieval
		kev_count++;
	}
	
	if (kev_count > 0 && kevent(_eventFd, kevs, kev_count, NULL, 0, NULL) == -1)
	{
		LOG_ERROR_ERRNO("kevent failed adding event for fd " << fd);
		return -1;
	}
	LOG_TRACE("Added " << kev_count << " event(s) for fd:" << fd << " udata:" << udata);
#endif

	if (needAddRead)
		udata->monitoringRead = true;
	if (needAddWrite)
		udata->monitoringWrite = true;
	return 0;
}

/**
 * Remove a file descriptor from the event loop
 *
 *
 * @param fd     File descriptor to stop monitoring
 * @param event  Type of event (IO_READ or IO_WRITE)
 *
 *
 * Note: On Linux, epoll removes the entire fd (not just one event type)
 *       On macOS/BSD, kqueue removes the specific filter
 */
int EventLoop::deleteEvent(int fd, int event, IOEventHandler *udata)
{
	if (_eventFd < 0)
		return (-1);

	// Check which events need to be deleted
	bool needDelRead = (event & IO_READ) && udata->monitoringRead;
	bool needDelWrite = (event & IO_WRITE) && udata->monitoringWrite;
	
	if (!needDelRead && !needDelWrite)
		return (0); // Nothing to delete

#if defined(__linux__)
	// Linux: Check if we're removing all events or just modifying
	bool removeAll = needDelRead && needDelWrite;
	
	if (removeAll)
	{
		// Remove entire fd from epoll
		if (epoll_ctl(_eventFd, EPOLL_CTL_DEL, fd, NULL) == -1)
			perror("epoll_ctl DEL");
	}
	else
	{
		// Modify to remove only specific event
		struct epoll_event ev;
		ev.events = 0;
		
		// Keep the events that are not being deleted
		if (udata->monitoringRead && !needDelRead)
			ev.events |= EPOLLIN;
		if (udata->monitoringWrite && !needDelWrite)
			ev.events |= EPOLLOUT;
		
		ev.data.ptr = udata;
		
		if (epoll_ctl(_eventFd, EPOLL_CTL_MOD, fd, &ev) == -1)
			perror("epoll_ctl MOD");
	}

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	// macOS/BSD: kqueue removes specific filter (READ or WRITE)
	// Can delete multiple filters in one call
	struct kevent kevs[2];
	int kev_count = 0;
	
	if (needDelRead)
	{
		EV_SET(&kevs[kev_count], fd, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
		kev_count++;
	}
	
	if (needDelWrite)
	{
		EV_SET(&kevs[kev_count], fd, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
		kev_count++;
	}
	
	if (kev_count > 0 && kevent(_eventFd, kevs, kev_count, NULL, 0, NULL) == -1)
		perror("kevent DEL");
#endif

	// std::cout << "Deleted event(s) for fd:" << fd << " udata:" << udata << std::endl;

	if (needDelRead)
		udata->monitoringRead = false;
	if (needDelWrite)
		udata->monitoringWrite = false;

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
 * 4. Use level-triggered I/O to avoid missing events
 */
void EventLoop::run()
{
	// TODO: add keepalive_timeout from config. and a simple queue to manage it
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
		addEvent((*it).second->getSocketfd(), IO_READ, (*it).second);
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
		// Check for timed-out clients before waiting for new events
		checkClientTimeouts();
		CheckChildProcesses();

#if defined(__linux__)
		// Linux: Wait for events with timeout for periodic cleanup
		// Returns number of triggered events
		int n = epoll_wait(_eventFd, events, maxevents, 1000); // 1 second timeout

		if (n == -1)
		{
			// EINTR means interrupted by signal (not a real error)
			if (errno == EINTR)
				continue; // Try again
			throw std::runtime_error("epoll_wait failed: " + std::string(strerror(errno)));
		}

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
		// macOS/BSD: Wait for events with timeout for periodic cleanup
		// Returns number of triggered events
		struct timespec timeout;
		timeout.tv_sec = 1; // 1 second timeout
		timeout.tv_nsec = 0;
		int n = kevent(_eventFd, NULL, 0, events, maxevents, &timeout);

		if (n == -1)
		{
			// EINTR means interrupted by signal (not a real error)
			if (errno == EINTR)
				continue; // Try again
			throw std::runtime_error("kevent failed: " + std::string(strerror(errno)));
		}
#endif
		// Process each triggered event
		for (int i = 0; i < n; i++)
		{
#if defined(__linux__)
			// Linux: Extract socket and event type from epoll event
			IOEventHandler *handler = (IOEventHandler *)events[i].data.ptr;
			if (!handler)
				continue;
			handler->events = mapEpollToIOFlags(events[i].events);

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
			// macOS/BSD: Extract socket and event type from kqueue event
			IOEventHandler *handler = (IOEventHandler *)events[i].udata;
			if (!handler)
				continue;
			// std::cout << "EventLoop: Processing event for handler " << handler << std::endl;
			handler->events = mapKqueueToIOFlags(events[i]);
#endif
			IOEventState state = handler->handleEvent(this);
			if (state == IO_EVENT_CONTINUE)
				continue;
			else if (state == IO_EVENT_CLOSE)
			{
				// std::cout << "EventLoop: Closing handler " << handler << std::endl;
				handler->handleDestroy(this);
				// delete handler;
			}
		} // End of event processing loop

		// Clean up all clients marked for deletion in this iteration
		// This prevents double-deletion issues on macOS where handleDestroy might be called twice
		for (std::list<ClientSocket *>::iterator it = _deleteClients.begin();
			 it != _deleteClients.end(); ++it)
		{
			ClientSocket *client = *it;
			delete client;
		}
		_deleteClients.clear();
	} // End of infinite while loop
}
