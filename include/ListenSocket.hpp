#pragma once

class ListenSocket;

#include "Socket.hpp"
#include "VirtualServer.hpp"
#include "ClientSocket.hpp"
#include "IOEventHandler.hpp"

class ClientSocket;

class ListenSocket : public Socket, public IOEventHandler
{
private:

	std::vector<VirtualServer *>	_servers;
	sockaddr_in				_addr;


	int 			_setOptions();
	ClientSocket 	*_accept();
	int 			_bind();
	int 			_listen();

public:

	ListenSocket(uint32_t ip, uint16_t port);
	~ListenSocket();

	int 	handleEvent(EventLoop *el);
	void    addVirtualServer(VirtualServer *virtualServer);
	const	std::vector<VirtualServer *> &getVirtualServers() const;

	sockaddr_in		getAddress() const;
	const char 		*getHost() const;
	int				getPort() const;

};

std::ostream &operator<<(std::ostream &os, const ListenSocket &ls);
