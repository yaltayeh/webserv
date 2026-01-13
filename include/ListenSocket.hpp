#pragma once

#include "Socket.hpp"
#include "ClientSocket.hpp"

class ClientSocket;

#define LISTEN_ID SOCKET_TYPE('L', 'S', 'T', 'N')

class ListenSocket : public Socket
{
private:
	uint32_t _ip;
	uint16_t _port;
	std::vector<VirtualServer *> servers;

public:
	ListenSocket(uint32_t ip, uint16_t port);
	~ListenSocket();

	int setOptions();
	ClientSocket *accept();
	int bind();
	int listen();

	void addVirtualServer(VirtualServer *virtualServer);
};
