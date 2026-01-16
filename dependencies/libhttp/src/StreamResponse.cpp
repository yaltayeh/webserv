#include "StreamResponse.hpp"

libhttp::StreamResponse::StreamResponse()
	: _fd(-1)
{
}

libhttp::StreamResponse::StreamResponse(int fd)
	: _fd(fd)
{
}

libhttp::StreamResponse::~StreamResponse()
{
}

int libhttp::StreamResponse::getFd() const
{
	return _fd;
}

void libhttp::StreamResponse::setFd(int fd)
{
	_fd = fd;
}
