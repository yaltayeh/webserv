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
	bool				_isHeadRequest;

	// // Timeout tracking: when this connection should be closed if inactive
	time_t _expiryDate;

public:

	ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen);
	~ClientSocket();

	// I/O Event Handling
	IOEventState 	handleEvent(EventLoop *el);
	void			handleDestroy(EventLoop *el);

	// HTTP handling
	libhttp::State			handleRead();
	bool					handleWrite();
	void			WriteToBuffer(char *buf, size_t len);
	
	// Timeout management
	time_t			getExpiryDate() const { return _expiryDate; }
	void			setExpiryDate(time_t expiry) { _expiryDate = expiry; }
	
	// Address info
	const char 		*getHost() const;
	int				getPort() const;
	sockaddr_in		getAddress() const;
};

std::ostream &operator<<(std::ostream &os, const ClientSocket &client);