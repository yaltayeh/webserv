#pragma once

#include "libhttp/include/Response.hpp"
#include <sys/types.h>
#include "StreamSocket.hpp"

class StreamSocket;

class StreamResponse : public libhttp::Response
{
private:
	std::string _buffer;
	bool _eof;
	StreamSocket *_streamSocket;

public:
	StreamResponse(int fd, int pid);
	~StreamResponse();
	
	StreamSocket *getStreamSocket() const;

	void appendToBuffer(const std::string &data, size_t size);

	size_t genChunk(char *buffer, size_t max_size);
	bool hasChunk() const;
	bool isComplete() const;

	friend class StreamSocket;
};
