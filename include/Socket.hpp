#pragma once

#include <sys/socket.h>
#include <map>

class Socket
{

protected:
	const unsigned int type;
	int _socketfd;

	Socket(unsigned int type, int socketfd);

public:
	virtual ~Socket() = 0;
	unsigned int getType() const;
	int setNonBlocking();
	void close();
	int getSocketfd() const;
	ssize_t recv(void *buf, size_t size);
	ssize_t send(const void *buf, size_t size);
};

#define SOCKET_TYPE(c0, c1, c2, c3)          \
	((unsigned int)((unsigned int)c0 << 0) | \
	 ((unsigned int)c2 << 8) |               \
	 ((unsigned int)c2 << 16) |              \
	 ((unsigned int)c3 << 24))
