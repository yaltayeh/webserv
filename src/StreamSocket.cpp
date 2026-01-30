#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>
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
		// Write request body to CGI
		if (!_response)
			return IO_EVENT_CONTINUE;
		
		// Check if we have body data to write
		if (_response->_bodyFileFd != -1)
		{
			// Read from temp file and write to socket
			char buf[4096];
			ssize_t nread = ::read(_response->_bodyFileFd, buf, sizeof(buf));
			
			if (nread < 0)
			{
				std::cerr << "Error reading from body temp file" << std::endl;
				::close(_response->_bodyFileFd);
				_response->_bodyFileFd = -1;
				// Stop monitoring for writes
				el->deleteEvent(_socketfd, IO_WRITE, this);
			}
			else if (nread == 0)
			{
				// EOF reached, close file
				::close(_response->_bodyFileFd);
				_response->_bodyFileFd = -1;
				// Stop monitoring for writes
				el->deleteEvent(_socketfd, IO_WRITE, this);
			}
			else
			{
				// Write to socket
				ssize_t nwritten = send(buf, nread);
				if (nwritten < 0)
				{
					return IO_EVENT_CLOSE;
				}
				else if (nwritten < nread)
				{
					// Partial write, seek back
					::lseek(_response->_bodyFileFd, -(nread - nwritten), SEEK_CUR);
				}
			}
		}
		else if (_response->_bodyWriteOffset < _response->_requestBody.size())
		{
			// Write from in-memory body
			const char *data = _response->_requestBody.c_str() + _response->_bodyWriteOffset;
			size_t remaining = _response->_requestBody.size() - _response->_bodyWriteOffset;
			
			ssize_t nwritten = send(data, remaining);
			if (nwritten < 0)
			{
				return IO_EVENT_CLOSE;
			}
			else
			{
				_response->_bodyWriteOffset += nwritten;
				
				// If all body written, stop monitoring for writes
				if (_response->_bodyWriteOffset >= _response->_requestBody.size())
				{
					el->deleteEvent(_socketfd, IO_WRITE, this);
				}
			}
		}
		else
		{
			// No body to write, stop monitoring for writes
			el->deleteEvent(_socketfd, IO_WRITE, this);
		}
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
