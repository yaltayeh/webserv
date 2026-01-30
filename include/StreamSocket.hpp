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
	libhttp::RequestBody _requestBody;

	std::string _writeBuffer;
	int _writeFd;  // Separate write file descriptor for CGI stdin
	
public:
	StreamSocket(int readFd, int writeFd, pid_t pid, StreamResponse *response, const libhttp::RequestBody &requestBody);
	~StreamSocket();

	int getWriteFd() const { return _writeFd; }

	IOEventState	handleEvent(EventLoop *el);
	void 			handleDestroy(EventLoop *el);

	ssize_t 		send(const void *buf, size_t size);
	void 			close();

	void handleChildExit(int status);

	friend class StreamResponse;
};
