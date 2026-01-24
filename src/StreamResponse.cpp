#include "StreamResponse.hpp"

StreamResponse::StreamResponse(int fd, int pid)
	: _eof(false), _streamSocket(NULL)
{
	_streamSocket = new StreamSocket(fd, pid, this);
}

#include <iostream>
StreamResponse::~StreamResponse()
{
	std::cout << "StreamResponse destructor: killing child process " << _streamSocket << std::endl;
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
	return !_buffer.empty();
}

bool StreamResponse::isComplete() const
{
	return _eof && _buffer.empty();
}
