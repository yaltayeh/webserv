#pragma once

#include <sys/socket.h>
#include <map>

class Socket
{
protected:

	int _socketfd;

	Socket(int socketfd);

public:

	virtual ~Socket() = 0;

	int 			setNonBlocking();
	int 			getSocketfd() const;
	virtual ssize_t recv(void *buf, size_t size);
	virtual ssize_t send(const void *buf, size_t size);
	virtual void 	close();
};
