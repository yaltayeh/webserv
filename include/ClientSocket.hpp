#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "libhttp/include/Parser.hpp"
#include "IOEventHandler.hpp"
// #include <chrono>
#include <ctime>

class ListenSocket;

class ClientSocket : public Socket, public IOEventHandler
{
private:

	const sockaddr_in		_addr;
	const ListenSocket 		*_listenSocket;
	
	std::string			_write_buffer;
	libhttp::Parser 	*_parser;
	
	libhttp::Response	*_response;

	// // Timeout tracking: when this connection should be closed if inactive
	time_t _expiryDate;

public:

	ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen);
	~ClientSocket();

	int 		handleEvent(EventLoop *el);
	sockaddr_in		getAddress() const;
	void			WriteToBuffer(char *buf, size_t len);
	const char 		*getHost() const;
	int				getPort() const;
	
	libhttp::State			handleRead();
	bool					handleWrite();

	// Timeout management
	time_t			getExpiryDate() const { return _expiryDate; }
	void			setExpiryDate(time_t expiry) { _expiryDate = expiry; }
};

std::ostream &operator<<(std::ostream &os, const ClientSocket &client);