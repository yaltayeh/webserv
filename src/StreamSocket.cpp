#include "StreamSocket.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>
#include "BufferConfig.hpp"

StreamSocket::StreamSocket(int readFd, int writeFd, pid_t pid, StreamResponse *response)
	: Socket(readFd), ChildProcessHandler(pid), _response(response), _writeFd(writeFd),
	  _writeBufferOffset(0)
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
		// Write request body to CGI stdin
		if (!_response)
			return IO_EVENT_CONTINUE;
		
		// Check if we have body data to write
		if (_response->_bodyFileFd != -1)
		{
			// First, try to write any buffered data from previous partial write
			if (_writeBufferOffset < _writeBuffer.size())
			{
				const char *data = _writeBuffer.c_str() + _writeBufferOffset;
				size_t remaining = _writeBuffer.size() - _writeBufferOffset;
				
				ssize_t nwritten = ::write(_writeFd, data, remaining);
				if (nwritten < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						// Pipe full, try again later
						return IO_EVENT_CONTINUE;
					}
					else
					{
						// Write error
						return IO_EVENT_CLOSE;
					}
				}
				else
				{
					_writeBufferOffset += nwritten;
					
					// If all buffered data written, clear buffer and continue reading from file
					if (_writeBufferOffset >= _writeBuffer.size())
					{
						_writeBuffer.clear();
						_writeBufferOffset = 0;
					}
					else
					{
						// Still have buffered data, try again later
						return IO_EVENT_CONTINUE;
					}
				}
			}
			
			// Read from temp file and write to pipe
			char buf[4096];
			ssize_t nread = ::read(_response->_bodyFileFd, buf, sizeof(buf));
			
			if (nread < 0)
			{
				std::cerr << "Error reading from body temp file" << std::endl;
				::close(_response->_bodyFileFd);
				_response->_bodyFileFd = -1;
				// Close write pipe to signal EOF to CGI
				if (_writeFd != -1)
				{
					el->deleteEvent(_writeFd, IO_WRITE, this);
					::close(_writeFd);
					_writeFd = -1;
				}
			}
			else if (nread == 0)
			{
				// EOF reached, close file and write pipe
				::close(_response->_bodyFileFd);
				_response->_bodyFileFd = -1;
				// Close write pipe to signal EOF to CGI
				if (_writeFd != -1)
				{
					el->deleteEvent(_writeFd, IO_WRITE, this);
					::close(_writeFd);
					_writeFd = -1;
				}
			}
			else
			{
				// Write to pipe
				ssize_t nwritten = ::write(_writeFd, buf, nread);
				if (nwritten < 0)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						// Pipe full, save all data to buffer for next try
						_writeBuffer.assign(buf, nread);
						_writeBufferOffset = 0;
					}
					else
					{
						// Write error
						return IO_EVENT_CLOSE;
					}
				}
				else if (nwritten < nread)
				{
					// Partial write, save remaining data to buffer
					_writeBuffer.assign(buf + nwritten, nread - nwritten);
					_writeBufferOffset = 0;
				}
				// else: all data written successfully, continue
			}
		}
		else if (_response->_bodyWriteOffset < _response->_requestBody.size())
		{
			// Write from in-memory body
			const char *data = _response->_requestBody.c_str() + _response->_bodyWriteOffset;
			size_t remaining = _response->_requestBody.size() - _response->_bodyWriteOffset;
			
			ssize_t nwritten = ::write(_writeFd, data, remaining);
			if (nwritten < 0)
			{
				if (errno != EAGAIN && errno != EWOULDBLOCK)
				{
					return IO_EVENT_CLOSE;
				}
				// Pipe full, try again later
			}
			else
			{
				_response->_bodyWriteOffset += nwritten;
				
				// If all body written, close write pipe to signal EOF
				if (_response->_bodyWriteOffset >= _response->_requestBody.size())
				{
					if (_writeFd != -1)
					{
						el->deleteEvent(_writeFd, IO_WRITE, this);
						::close(_writeFd);
						_writeFd = -1;
					}
				}
			}
		}
		else
		{
			// No body to write, close write pipe
			if (_writeFd != -1)
			{
				el->deleteEvent(_writeFd, IO_WRITE, this);
				::close(_writeFd);
				_writeFd = -1;
			}
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
	
	// Clean up write fd if it's different from read fd
	if (_writeFd != -1 && _writeFd != _socketfd)
	{
		el->deleteEvent(_writeFd, IO_WRITE, this);
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

void StreamSocket::handleChildExit(int status)
{
	(void)status;
	if (_response)
		_response->_eof = 1;
	_pid = -1;
}
