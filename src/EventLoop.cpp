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
	(void)event; // Unused on Linux
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
		addEvent((*it).second->getSocketfd(), EVENT_READ, (*it).second);
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
							break; // No more connections to accept
						perror("accept");
						break;
					}

					// Add client socket to event loop (monitor for reads)
					addEvent(clientSocket->getSocketfd(), EVENT_READ, clientSocket);
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
					deleteEvent(client->getSocketfd(), EVENT_READ);
					delete client;
					continue;
				}

				// Handle read event
				if (is_read)
				{
					if (client->handleRead() == false)
					{
						deleteEvent(client->getSocketfd(), EVENT_READ);
						delete client;
						continue;
					}

					// If response is ready, switch to monitoring for write
					if (client->isComplateResponse())
					{
						deleteEvent(client->getSocketfd(), EVENT_READ);
						addEvent(client->getSocketfd(), EVENT_WRITE, client);
					}
				}

				// Handle write event
				else if (is_write)
				{
					client->handleWrite();

					// If response fully sent, clean up client
					if (client->isSendResponse())
					{
						deleteEvent(client->getSocketfd(), EVENT_WRITE);
						delete client;
					}
				}
			}
		}
	}
	return (0);
}
