#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "libhttp/include/Parser.hpp"
#include "IOEventHandler.hpp"
#include <chrono>

class ListenSocket;

class ClientSocket : public Socket, public IOEventHandler
{
private:

	const sockaddr_in		_addr;
	const ListenSocket 		*_listenSocket;

	// From ConnectionManager (struct Connection)
	std::string			_write_buffer;
	libhttp::Parser 	*_parser;

	int		_isRequestRead;
	int		_isResponseSent;

	std::chrono::system_clock::time_point expiryDate;

public:

	ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen);
	~ClientSocket();

	int 			handleEvent(EventLoop *el);
	sockaddr_in		getAddress();
	const char 		*getHost();
	void			WriteToBuffer(char *buf, size_t len);
	bool			handleRead();
	bool			handleWrite();

};
