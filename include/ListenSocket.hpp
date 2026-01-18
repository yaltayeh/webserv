#pragma once

class ListenSocket;

#include "Socket.hpp"
#include "VirtualServer.hpp"
#include "ClientSocket.hpp"
#include "EventData.hpp"

class ClientSocket;

class ListenSocket : public Socket, public EventData
{
private:
	uint32_t _ip;
	uint16_t _port;
	std::vector<VirtualServer *> _servers;

	int _setOptions();
	ClientSocket *_accept();
	int _bind();
	int _listen();

public:
	ListenSocket(uint32_t ip, uint16_t port);
	~ListenSocket();
	int EventHandler(EventLoop *el);

	void addVirtualServer(VirtualServer *virtualServer);
	const std::vector<VirtualServer *> &getVirtualServer() const;
};
