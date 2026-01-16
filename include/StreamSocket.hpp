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
	std::string &_buffer;

public:
	StreamSocket(int fd, std::string &buffer);
	~StreamSocket();

	int EventHandler(EventLoop *el);
};
