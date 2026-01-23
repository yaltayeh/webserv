#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>

StreamSocket::StreamSocket(int fd, ClientSocket *clientSocket)
	: Socket(fd), _clientSocket(clientSocket) {}

StreamSocket::~StreamSocket() {}

IOEventState StreamSocket::handleEvent(EventLoop *el)
{
	(void)el;
	
	// --- Sub-case A: StreamSocket Disconnected ---
	if (events & (IO_ERROR | IO_HUP))
	{
		std::cout << "StreamSocket for client " << *_clientSocket << " disconnected" << std::endl;
		return IO_EVENT_CLOSE;
	}
	if (events & IO_READ)
	{
		char buf[4096]; // TODO: unify buffer size for read and write in all places

		ssize_t nbytes = recv(buf, sizeof(buf));
		if (nbytes <= 0)
			return IO_EVENT_CLOSE;
		_clientSocket->WriteToBuffer(buf, nbytes);
	}
	return IO_EVENT_CONTINUE;
}

void StreamSocket::handleDestroy(EventLoop *el)
{
	el->deleteEvent(_socketfd, IO_READ, this);
	el->deleteEvent(_socketfd, IO_WRITE, this);
}
