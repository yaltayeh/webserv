#include "StreamResponse.hpp"

libhttp::StreamResponse::StreamResponse()
	: _fd(-1), _pid(-1) { }
libhttp::StreamResponse::StreamResponse(int fd, int pid)
	: _fd(fd), _pid(pid) { }
libhttp::StreamResponse::~StreamResponse() { }


int libhttp::StreamResponse::getFd() const
{
	return _fd;
}

int libhttp::StreamResponse::getPid() const
{
	return _pid;
}
