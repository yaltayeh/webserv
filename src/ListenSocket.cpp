#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include "ClientSocket.hpp"
#include "ListenSocket.hpp"
#include "EventLoop.hpp"
#include "libutils/include/Logger.hpp"

// Helper to convert IP address to string (replaces inet_ntoa which is not allowed)
static std::string ipToString(uint32_t ip) {
	std::ostringstream oss;
	unsigned char *bytes = (unsigned char *)&ip;
	oss << (int)bytes[0] << "." << (int)bytes[1] << "." << (int)bytes[2] << "." << (int)bytes[3];
	return oss.str();
}

ListenSocket::ListenSocket(uint32_t ip, uint16_t port)
	: Socket(-1)
{
	/*
	 * Initialize the sockaddr_in structure with the provided IP and port.
	 * This structure will be used for binding the listening socket.
	 */
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port);
	_addr.sin_addr.s_addr = ip;

	if (ListenSocket::_setOptions() < 0)
	{
		close();
		std::ostringstream oss;
		oss << "Failed to set socket options on " << ipToString(ip) << ":" << port;
		throw std::runtime_error(oss.str());
	}
	if (ListenSocket::_bind() < 0)
	{
		close();
		std::ostringstream oss;
		oss << "Failed to bind socket to " << ipToString(ip) << ":" << port;
		throw std::runtime_error(oss.str());
	}
	if (ListenSocket::_listen() < 0)
	{
		close();
		std::ostringstream oss;
		oss << "Failed to listen on " << ipToString(ip) << ":" << port;
		throw std::runtime_error(oss.str());
	}
}

ListenSocket::~ListenSocket() { }

IOEventState ListenSocket::handleEvent(EventLoop *el)
{
	while (1)
	{
		if (el->getConfig().http.worker_connections > 0 &&
			el->getClientQueue().size() >= (size_t)el->getConfig().http.worker_connections)
		{
			// Reached max connections, stop accepting new clients
			LOG_WARNING("Max worker_connections reached (" << el->getConfig().http.worker_connections << "), not accepting new clients");
			return IO_EVENT_CONTINUE;
		}
		// Try to accept a new client connection
		ClientSocket *clientSocket = _accept();

		if (!clientSocket)
		{
			// EAGAIN/EWOULDBLOCK means no more connections (normal)
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return IO_EVENT_CONTINUE;

			LOG_ERROR_ERRNO("accept() failed on listening socket");
			return IO_EVENT_CLOSE;
		}
		el->markClientActive(clientSocket);
		// Add new client to event loop (start monitoring for HTTP request)
		el->addEvent(clientSocket->getSocketfd(), IO_READ | IO_WRITE, clientSocket);
		LOG_DEBUG("Added client to event loop: " << *clientSocket);
	}
	return IO_EVENT_CONTINUE;
}

void ListenSocket::handleDestroy(EventLoop *el)
{
	el->deleteEvent(_socketfd, IO_READ | IO_WRITE, this);
}

int ListenSocket::_setOptions()
{
	/**
	 * ========================================================================
	 * STEP 2: Set SO_REUSEADDR socket option
	 * ========================================================================
	 *
	 * WHY THIS IS ESSENTIAL:
	 * When a server stops, the OS keeps the port in TIME_WAIT state for
	 * ~60 seconds. Without SO_REUSEADDR, restarting the server within that
	 * window fails with "Address already in use". This option allows
	 * immediate rebinding, which is critical for development.
	 *
	 * setsockopt() parameters:
	 * - _listen_fd: Socket to configure
	 * - SOL_SOCKET: Set option at socket API level (not TCP/IP level)
	 * - SO_REUSEADDR: Allow reuse of local addresses
	 * - &opt: Pointer to option value (1 = enable)
	 * - sizeof(opt): Size of option value
	 */
	int opt = 1;
	if (setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		LOG_ERROR_ERRNO(*this << " Failed to set SO_REUSEADDR");
		Socket::close();
		return (-1);
	}
	return (0);
}

int ListenSocket::_bind()
{
	/**
	 * ========================================================================
	 * STEP 3: Bind socket to address (IP + port)
	 * ========================================================================
	 *
	 * WHAT bind() DOES:
	 * Associates the socket with a specific network address and port.
	 * After binding, the socket can receive connections on that address.
	 *
	 * sockaddr_in structure:
	 * - sin_family: Address family (AF_INET = IPv4)
	 * - sin_port: Port number in network byte order (htons() converts)
	 * - sin_addr.s_addr: IP address (INADDR_ANY = 0.0.0.0 = all interfaces)
	 *
	 * WHY INADDR_ANY:
	 * Binds to all available network interfaces. This means the server
	 * accepts connections on localhost (127.0.0.1), LAN IP (192.168.x.x),
	 * and public IP if available. For development, this is convenient.
	 *
	 * FUTURE ENHANCEMENT:
	 * Use _config.host instead of INADDR_ANY to support binding to
	 * specific interfaces. This requires inet_pton() to convert string
	 * IP to binary format.
	 */
	int err = ::bind(_socketfd, (sockaddr *)&_addr, sizeof(_addr));
	if (err < 0)
	{
		LOG_CRITICAL_ERRNO(*this << " Failed to bind socket");
		Socket::close();
	}

	return (err);
}

int ListenSocket::_listen()
{
	/**
	 * ========================================================================
	 * STEP 4: Start listening for connections
	 * ========================================================================
	 *
	 * WHAT listen() DOES:
	 * Marks the socket as passive (ready to accept connections). Creates
	 * a queue for pending connections with specified backlog size.
	 *
	 * BACKLOG PARAMETER (512):
	 * Maximum number of pending connections in the queue. If queue is
	 * full, new connection attempts are rejected. 512 is a standard value
	 * that works well for most servers.
	 *
	 * AFTER listen():
	 * - Socket is ready to accept() connections
	 * - Kernel starts accepting SYN packets (TCP handshake)
	 * - Completed connections wait in queue until accept() is called
	 */
	int err = ::listen(_socketfd, 512);
	if (err < 0)
	{
		LOG_CRITICAL_ERRNO(*this << " Failed to listen");
		Socket::close();
	}
	/**
	 * ========================================================================
	 * SUCCESS: Listening socket is ready
	 * ========================================================================
	 */
	LOG_INFO(*this << " is now listening");
	return err;
}

ClientSocket *ListenSocket::_accept()
{
	/**
	 * accept() parameters:
	 * - _listen_fd: Listening socket with pending connections
	 * - &client_addr: Pointer to structure that receives client's address
	 * - &addr_len: Size of address structure (modified by accept())
	 *
	 * accept() returns:
	 * - New socket fd for this specific client (>= 0)
	 * - -1 on error or no connections pending (check errno)
	 *
	 * WHAT HAPPENS:
	 * - Removes one connection from the listening socket's queue
	 * - Creates a brand NEW socket for this client
	 * - Returns the new socket's file descriptor
	 * - Listening socket remains open and keeps listening
	 */
	sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	int client_fd = ::accept(_socketfd, (sockaddr *)&client_addr, &addr_len);
	if (client_fd < 0)
	{
		/**
		 * Note: errno is not checked per 42 subject rules, but accept()
		 * returning -1 in non-blocking mode usually means EAGAIN (no
		 * connections pending). This is normal and not a real error.
		 */
		return NULL;
	}

	ClientSocket *clientSocket = new ClientSocket(client_fd, client_addr, this);

	/**
	 * Log successful connection (useful for debugging)
	 */
	LOG_INFO(*this << " accepted " << *clientSocket);

	return clientSocket;
}

void ListenSocket::addVirtualServer(VirtualServer *virtualServer)
{
	_servers.push_back(virtualServer);
}

const std::vector<VirtualServer *> &ListenSocket::getVirtualServers() const
{
	return (_servers);
}

sockaddr_in ListenSocket::getAddress() const
{
	return _addr;
}

const char *ListenSocket::getHost() const
{
	// Cannot use inet_ntoa (not in allowed functions list)
	// Use helper function instead
	static std::string addr_str;
	addr_str = ipToString(_addr.sin_addr.s_addr);
	return addr_str.c_str();
}

int ListenSocket::getPort() const
{
	return ntohs(_addr.sin_port);
}

std::ostream &operator<<(std::ostream &os, const ListenSocket &ls)
{
	os << "ListenSocket(fd=" << ls.getSocketfd()
	   << ", addr=" << ls.getHost()
	   << ":" << ls.getPort() << ")";
	return os;
}
