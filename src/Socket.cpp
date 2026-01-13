#include <iostream>
#include <unistd.h>
#include "Socket.hpp"
#include <fcntl.h>
#include <cerrno>
#include <cstring>

Socket::Socket(unsigned int type, int socketfd)
	: type(type), _socketfd(socketfd)
{
	/**
	 * ========================================================================
	 * STEP 1: Create TCP socket (IPv4, stream-based)
	 * ========================================================================
	 *
	 * socket() parameters:
	 * - AF_INET: IPv4 protocol
	 * - SOCK_STREAM: TCP (reliable, connection-oriented)
	 * - 0: Let system choose protocol (TCP for SOCK_STREAM)
	 */
	if (_socketfd == -1)
	{
		_socketfd = socket(AF_INET, SOCK_STREAM, 0);
		if (_socketfd < 0)
		{
			std::cerr << "Error: Failed to create socket: "
					  << strerror(errno) << std::endl;
			Socket::close();
		}
	}

	/**
	 * ========================================================================
	 * STEP 5: Set socket to non-blocking mode
	 * ========================================================================
	 *
	 * WHY THIS IS MANDATORY:
	 * The 42 subject requires non-blocking I/O with poll(). This ensures:
	 * - accept() never blocks (returns -1 with EAGAIN if no connections)
	 * - read() never blocks (returns -1 with EAGAIN if no data)
	 * - write() never blocks (returns -1 with EAGAIN if buffer full)
	 *
	 * Combined with poll(), this gives us full control over I/O timing.
	 */
	if (setNonBlocking() < 0)
	{
		std::cerr << "Error: Failed to set non-blocking mode on "
				  << strerror(errno) << std::endl;
		Socket::close();
	}
}

Socket::~Socket()
{
	Socket::close();
}

unsigned int Socket::getType() const
{
	return type;
}

/**
 * Sets a file descriptor to non-blocking mode
 *
 * WHAT THIS DOES:
 * Uses fcntl() to add the O_NONBLOCK flag to the socket, which makes
 * all I/O operations on this socket non-blocking (read/write/accept
 * return immediately instead of waiting).
 *
 * WHY THIS IS CRITICAL:
 * The 42 subject requires all I/O to go through poll(). Non-blocking
 * sockets ensure we never freeze waiting for I/O - we only read/write
 * when poll() tells us it's safe.
 *
 * PARAMETERS:
 * - fd: File descriptor to make non-blocking
 *
 * RETURNS:
 * - 0 on success
 * - -1 on failure
 */
int Socket::setNonBlocking()
{
	/**
	 * Get current flags on the file descriptor
	 */
	int flags = fcntl(_socketfd, F_GETFL, 0);
	if (flags < 0)
	{
		return -1;
	}

	/**
	 * Add O_NONBLOCK flag and set it back
	 */
	if (fcntl(_socketfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return -1;
	}

	return 0;
}

void Socket::close()
{
	if (_socketfd >= 0)
		::close(_socketfd);
	_socketfd = -1;
}

int Socket::getSocketfd() const
{
	return _socketfd;
}

ssize_t Socket::recv(void *buf, size_t size)
{
	return ::recv(_socketfd, buf, size, 0);
}

ssize_t Socket::send(const void *buf, size_t size)
{
	return ::send(_socketfd, buf, size, 0);
}
