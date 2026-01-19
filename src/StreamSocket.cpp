#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>

StreamSocket::StreamSocket(int fd, ClientSocket *clientSocket)
	: Socket(fd), _clientSocket(clientSocket)
{
}

StreamSocket::~StreamSocket()
{
}

int StreamSocket::handleEvent(EventLoop *el)
{
	if (events & (IO_ERROR | IO_HUP))
	{
		el->deleteEvent(_socketfd, IO_READ, this);
		el->deleteEvent(_socketfd, IO_WRITE, this);
		delete this;
	}
	if (events & IO_READ)
	{
		char buf[4096];

		ssize_t nbytes = recv(buf, sizeof(buf));
		if (nbytes <= 0)
			return (1);
		_clientSocket->WriteToBuffer(buf, nbytes);
		el->addEvent(_clientSocket->getSocketfd(), IO_WRITE, _clientSocket);
	}
	return 0;
}
