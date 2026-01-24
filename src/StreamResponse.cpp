#include "StreamResponse.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "libhttp/include/Utils.hpp"

StreamResponse::StreamResponse(int fd, int pid)
	: _eof(false), _streamSocket(NULL), _findStatusLine(false)
{
	_streamSocket = new StreamSocket(fd, pid, this);
}
StreamResponse::~StreamResponse()
{
	if (_streamSocket)
	{
		_streamSocket->_response = NULL;
		_streamSocket->killChild(SIGKILL);
		// StreamSocket will be deleted when handleDestroy is called
		// or we need to delete it here if it's not managed by EventLoop
		// delete _streamSocket;
		_streamSocket = NULL;
	}
}

StreamSocket *StreamResponse::getStreamSocket() const
{
	return _streamSocket;
}

void StreamResponse::appendToBuffer(const std::string &data, size_t size)
{
	if (_buffer.capacity() < _buffer.size() + size)
		_buffer.reserve(_buffer.size() + size);
	_buffer.append(data.c_str(), size);

	if (!_findStatusLine)
	{
		size_t status_pos = _buffer.find("Status:");
		if (status_pos == std::string::npos)
			status_pos = _buffer.find("status:");
		
		// Check if we have complete headers
		size_t header_end = _buffer.find("\r\n\r\n");
		bool use_crlf = true;
		if (header_end == std::string::npos)
		{
			header_end = _buffer.find("\n\n");
			use_crlf = false;
		}
		
		if (status_pos != std::string::npos)
		{
			// Found Status header, parse it
			size_t line_end = _buffer.find(use_crlf ? "\r\n" : "\n", status_pos);
			if (line_end != std::string::npos)
			{
				std::string status_line = _buffer.substr(status_pos, line_end - status_pos);
				// Extract status code and reason phrase
				size_t colon_pos = status_line.find(':');
				if (colon_pos != std::string::npos)
				{
					std::string status_value = status_line.substr(colon_pos + 1);
					// Trim leading spaces
					size_t first = status_value.find_first_not_of(" \t");
					if (first != std::string::npos)
						status_value = status_value.substr(first);
					
					// Create HTTP status line
					std::string http_status = "HTTP/1.1 " + status_value + "\r\n";
					
					// Remove the Status header line from buffer
					_buffer.erase(status_pos, line_end - status_pos + (use_crlf ? 2 : 1));
					
					// Insert HTTP status line at the beginning
					_buffer.insert(0, http_status);
					
					_findStatusLine = true;
				}
			}
		}
		else if (header_end != std::string::npos)
		{
			// No Status header found but headers are complete, use default 200
			std::string http_status = "HTTP/1.1 200 OK\r\n";
			_buffer.insert(0, http_status);
			_findStatusLine = true;
		}
	}
}

size_t StreamResponse::genChunk(char *buffer, size_t max_size)
{
	size_t to_copy = std::min(max_size, _buffer.size());
	std::memcpy(buffer, _buffer.c_str(), to_copy);
	_buffer.erase(0, to_copy);
	return to_copy;
}

bool StreamResponse::hasChunk() const
{
	return !_buffer.empty() && _findStatusLine;
}

bool StreamResponse::isComplete() const
{
	return _eof && _buffer.empty();
}
