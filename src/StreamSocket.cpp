#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>
#include "BufferConfig.hpp"
#include "Logger.hpp"

StreamSocket::StreamSocket(int readFd, int writeFd, pid_t pid, StreamResponse *response, const libhttp::RequestBody &requestBody)
	: Socket(readFd), ChildProcessHandler(pid), _response(response),
	  _requestBody(requestBody), _writeFd(writeFd)
{
}

StreamSocket::~StreamSocket()
{
	if (_writeFd != -1 && _writeFd != _socketfd)
	{
		::close(_writeFd);
		_writeFd = -1;
	}

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
		LOG_DEBUG("StreamSocket::handleEvent IO_WRITE " << _writeFd);
		if (!_writeBuffer.empty())
		{
			ssize_t nbytes = send(_writeBuffer.c_str(), _writeBuffer.size());
			if (nbytes < 0)
				return IO_EVENT_CLOSE;
			if (nbytes == 0)
				return IO_EVENT_CONTINUE;
			_writeBuffer.erase(0, nbytes);
			if (!_writeBuffer.empty())
				return IO_EVENT_CONTINUE;
		}

		char buf[WRITE_BUFFER_SIZE];
		ssize_t chunkSize = _requestBody.getChunk(buf, sizeof(buf));
		if (chunkSize < 0)
			return IO_EVENT_CLOSE;
		if (chunkSize == 0)
		{
			// No more data to read from body
			// Close write FD to signal EOF to CGI script
			el->deleteEvent(_writeFd, IO_WRITE, this);
			::close(_writeFd);
			_writeFd = -1;
			return IO_EVENT_CONTINUE;
		}
		ssize_t nbytes = send(buf, chunkSize);
		if (nbytes < 0)
			return IO_EVENT_CLOSE;
		if (nbytes == 0)
			return IO_EVENT_CONTINUE;
		if (nbytes != chunkSize)
			_writeBuffer.append(buf + nbytes, chunkSize - nbytes);
	}
	if (events & IO_READ)
	{
		char buf[READ_BUFFER_SIZE];

		ssize_t nbytes = recv(buf, sizeof(buf));
		LOG_DEBUG("StreamSocket::handleEvent IO_READ socket=" << _socketfd << " nbytes=" << nbytes << " errno=" << strerror(errno));
		if (nbytes <= 0)
			return IO_EVENT_CLOSE;
		if (_response)
			_response->appendToBuffer(buf, nbytes);
	}
	return IO_EVENT_CONTINUE;
}

void StreamSocket::handleDestroy(EventLoop *el)
{
	if (_socketfd != -1)
		el->deleteEvent(_socketfd, IO_READ, this);
	
	if (_response)
	{
		_response->_eof = 1;
		_response->_streamSocket = NULL;
		_response = NULL;
	}
}

ssize_t StreamSocket::send(const void *buf, size_t size)
{
	return ::send(_writeFd, buf, size, MSG_NOSIGNAL);
}

void StreamSocket::close()
{
	Socket::close();
	if (_writeFd != -1)
		::close(_writeFd);
	_writeFd = -1;
}

void StreamSocket::handleChildExit(int status)
{
	(void)status;
	if (_response)
		_response->_eof = 1;
	_pid = -1;
}
