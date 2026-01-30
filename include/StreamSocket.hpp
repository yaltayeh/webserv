#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include <string>
#include "IOEventHandler.hpp"
#include "StreamResponse.hpp"
#include "StreamSocket.hpp"
#include "ChildProcessHandler.hpp"

class StreamResponse;

class StreamSocket : public Socket, public IOEventHandler, public ChildProcessHandler
{
private:
	StreamResponse *_response;
	int _writeFd;  // Separate write file descriptor for CGI stdin
	
	// Buffer for handling partial writes from body file
	std::string _writeBuffer;
	size_t _writeBufferOffset;

public:
	StreamSocket(int readFd, int writeFd, pid_t pid, StreamResponse *response);
	~StreamSocket();

	int getWriteFd() const { return _writeFd; }

	IOEventState	handleEvent(EventLoop *el);
	void 			handleDestroy(EventLoop *el);

	void handleChildExit(int status);

	friend class StreamResponse;
};
