#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "libhttp/include/Parser.hpp"
#include "IOEventHandler.hpp"
#include "ClientSocket.hpp"

class StreamSocket : public Socket, public IOEventHandler
{
private:

	// Non-owning pointer back to ClientSocket (ClientSocket owns this StreamSocket)
	ClientSocket*	_clientSocket;

public:

	// Constructor takes raw pointer - does NOT take ownership of ClientSocket
	StreamSocket(int fd, ClientSocket* clientSocket);
	~StreamSocket();

	int		handleEvent(EventLoop *el);

};
