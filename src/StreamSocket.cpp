#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include "BufferConfig.hpp"

StreamSocket::StreamSocket(int fd, pid_t pid, StreamResponse *response)
	: Socket(fd), ChildProcessHandler(pid), _response(response)
{
}

StreamSocket::~StreamSocket()
{
	if (_response)
	{
		_response->_eof = 1;
		_response->_streamSocket = NULL;
		_response = NULL;
	}
}

IOEventState StreamSocket::handleEvent(EventLoop *el)
{
	(void)el;

	// std::cout << "Event: " << events << std::endl;
	// --- Sub-case A: StreamSocket Disconnected ---
	if (events & (IO_ERROR | IO_HUP))
		return IO_EVENT_CLOSE;
	if (events & IO_WRITE)
	{
		(void)0; // TODO: Writing not implemented yet
	}
	if (events & IO_READ)
	{
		char buf[READ_BUFFER_SIZE];

		ssize_t nbytes = recv(buf, sizeof(buf));
		if (nbytes <= 0)
			return IO_EVENT_CLOSE;
		if (_response)
			_response->appendToBuffer(buf, nbytes);
	}
	return IO_EVENT_CONTINUE;
}

void StreamSocket::handleDestroy(EventLoop *el)
{
	// std::cout << "StreamSocket::handleDestroy called for fd " << _socketfd << std::endl;
	el->deleteEvent(_socketfd, IO_READ | IO_WRITE, this);
	if (_response)
	{
		_response->_eof = 1;
		_response->_streamSocket = NULL;
		_response = NULL;
	}
}

void StreamSocket::handleChildExit(int status)
{
	(void)status;
	if (_response)
		_response->_eof = 1;
	_pid = -1;
}
