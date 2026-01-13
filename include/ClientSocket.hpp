#pragma once

#include "Socket.hpp"
#include <netinet/in.h>
#include "ListenSocket.hpp"
#include "VirtualServer.hpp"
#include "libhttp/include/Parser.hpp"

class ListenSocket;

#define CLIENT_ID SOCKET_TYPE('C', 'L', 'N', 'T')

class ClientSocket : public Socket
{
private:
	const sockaddr_in addr;
	const ListenSocket *listenSocket;

	// From ConnectionManager (struct Connection)
	std::string write_buffer;
	libhttp::Parser parser;

public:
	ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen);
	~ClientSocket();
	sockaddr_in getAddress();
	const char *getHost();
	bool handleRead();
	bool handleWrite();

};
