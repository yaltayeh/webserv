#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "libhttp/include/Parser.hpp"
#include "EventData.hpp"

class CGISocket : public Socket, public EventData
{
private:

public:
	CGISocket();
	~CGISocket();
	int EventHandler(EventLoop *el);
};
