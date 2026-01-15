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
#include "ClientSocket.hpp"
#include "ListenSocket.hpp"
#include "EventLoop.hpp"

ListenSocket::ListenSocket(uint32_t ip, uint16_t port)
	: Socket(-1), _ip(ip), _port(port)
{
	if (setOptions())
	{
		close();
		return;
	} // TODO replace with throw
	if (ListenSocket::bind() < 0)
	{
		close();
		return;
	} // TODO replace with throw
	if (ListenSocket::listen() < 0)
	{
		close();
		return;
	} // TODO replace with throw
}

ListenSocket::~ListenSocket()
{
}

int ListenSocket::EventHandler(EventLoop *el)
{
	// Accept ALL pending connections (important for edge-triggered mode)
	// We must accept until EAGAIN, otherwise we might miss connections
	while (1)
	{
		// Try to accept a new client connection
		ClientSocket *clientSocket = accept();

		if (!clientSocket)
		{
			// EAGAIN/EWOULDBLOCK means no more connections (normal)
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			std::cerr << "Error: accept() failed: "
					  << strerror(errno) << std::endl;
			break;
		}

		// Add new client to event loop (start monitoring for HTTP request)
		el->addEvent(clientSocket->getSocketfd(), EVENT_READ, clientSocket);
		std::cout << "add client " << clientSocket->getHost() << std::endl;
	}
	return 0;
}

int ListenSocket::setOptions()
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
		std::cerr << "Error: Failed to set SO_REUSEADDR on "
				  << strerror(errno) << std::endl;
		Socket::close();
		return (-1);
	}
	return (0);
}

int ListenSocket::bind()
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
	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port); // FIX
	addr.sin_addr.s_addr = _ip;

	std::cout << "fd: " << _socketfd << std::endl;

	int err = ::bind(_socketfd, (sockaddr *)&addr, sizeof(addr));
	if (err < 0)
	{
		std::cerr << "Error: Failed to bind socket to port "
				  << _ip << ":" << _port << " (errno: " << strerror(errno) << ")" << std::endl;
		Socket::close();
	}

	return (err);
}

int ListenSocket::listen()
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
	 * BACKLOG PARAMETER (128):
	 * Maximum number of pending connections in the queue. If queue is
	 * full, new connection attempts are rejected. 128 is a standard value
	 * that works well for most servers.
	 *
	 * AFTER listen():
	 * - Socket is ready to accept() connections
	 * - Kernel starts accepting SYN packets (TCP handshake)
	 * - Completed connections wait in queue until accept() is called
	 */
	int err = ::listen(_socketfd, 128);
	if (err < 0)
	{
		std::cerr << "Error: Failed to listen on "
				  << _ip << _port << std::endl;
		Socket::close();
	}
	/**
	 * ========================================================================
	 * SUCCESS: Listening socket is ready
	 * ========================================================================
	 */
	std::cout << "Socket listening on "
			  << _ip << ":" << _port
			  << " (fd " << _socketfd << ")" << std::endl;

	return err;
}

ClientSocket *ListenSocket::accept()
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

	std::cout << "accept: fd: " << _socketfd << std::endl;
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
	std::cout << "client address: " << clientSocket->getHost() << std::endl;
	std::cout << "Socket on " << _ip << ":" << _port
			  << " accepted connection (fd " << client_fd << ")"
			  << std::endl;

	return clientSocket;
}

void ListenSocket::addVirtualServer(VirtualServer *virtualServer)
{
	servers.push_back(virtualServer);
}

const std::vector<VirtualServer *> &ListenSocket::getVirtualServer() const
{
	return (servers);
}
