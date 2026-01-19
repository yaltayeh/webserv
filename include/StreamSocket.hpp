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

	ClientSocket	*_clientSocket;

public:

	StreamSocket(int fd, ClientSocket *clientSocket);
	~StreamSocket();

	int		handleEvent(EventLoop *el);

};
