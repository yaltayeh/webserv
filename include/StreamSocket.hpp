#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "IOEventHandler.hpp"
#include "StreamResponse.hpp"
#include "StreamSocket.hpp"
#include "ChildProcessHandler.hpp"

class StreamResponse;

class StreamSocket : public Socket, public IOEventHandler, public ChildProcessHandler
{
private:
	StreamResponse *_response;

public:
	StreamSocket(int fd, pid_t pid, StreamResponse *response);
	~StreamSocket();

	IOEventState	handleEvent(EventLoop *el);
	void 			handleDestroy(EventLoop *el);

	void handleChildExit(int status);

	friend class StreamResponse;
};
