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
	void 			close();
	int 			getSocketfd() const;
	ssize_t 		recv(void *buf, size_t size);
	ssize_t 		send(const void *buf, size_t size);

};
