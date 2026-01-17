#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "libhttp/include/Parser.hpp"
#include "EventData.hpp"
#include "ClientSocket.hpp"

class StreamSocket : public Socket, public EventData
{
private:
	ClientSocket *_clientSocket;

public:
	StreamSocket(int fd, ClientSocket *clientSocket);
	~StreamSocket();

	int EventHandler(EventLoop *el);
};
