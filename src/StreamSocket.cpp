#include "StreamSocket.hpp"
#include "EventLoop.hpp"

StreamSocket::StreamSocket(int fd, std::string &buffer)
	: Socket(fd), _buffer(buffer)
{
}

StreamSocket::~StreamSocket()
{
}

int StreamSocket::EventHandler(EventLoop *el)
{
	if (isEOF)
	{
		el->deleteEvent(_socketfd, EVENT_READ);
		el->deleteEvent(_socketfd, EVENT_WRITE);
		delete this;
	}
	if (isRead)
	{
		char buf[4096];

		ssize_t nbytes;
		if ((nbytes = recv(buf, sizeof(buf))) <= 0)
			return (1);
		_buffer.append(buf, nbytes);
	}
	return 0;
}
