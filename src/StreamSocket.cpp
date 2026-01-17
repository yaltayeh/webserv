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

int StreamSocket::EventHandler(EventLoop *el)
{
	if (isEOF)
	{
		el->deleteEvent(_socketfd, EVENT_READ, this);
		el->deleteEvent(_socketfd, EVENT_WRITE, this);
		delete this;
	}
	if (isRead)
	{
		char buf[4096];

		ssize_t nbytes = recv(buf, sizeof(buf));
		if (nbytes <= 0)
			return (1);
		_clientSocket->WriteToBuffer(buf, nbytes);
		el->addEvent(_clientSocket->getSocketfd(), EVENT_WRITE, _clientSocket);
	}
	return 0;
}
