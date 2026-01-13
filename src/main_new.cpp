#include <iostream>
#include <vector>

#include "libdir/include/Parser.hpp"
#include "libdir/include/Config.hpp"
#include "Config/GlobalConfig.hpp"
#include "Config/ServerConfig.hpp"

#include "poll.h"
#include <sys/socket.h>
#include <Socket.hpp>
#include <ListenSocket.hpp>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include "VirtualServer.hpp"

void printUsage(const char *progName)
{
	std::cout << "Usage: " << progName << " <config-file>" << std::endl;
}

int event_loop(std::map<uint64_t, ListenSocket *> &sockets)
{
	int kq = kqueue();

	struct kevent kev;

	for (std::map<uint64_t, ListenSocket *>::iterator it = sockets.begin();
		 it != sockets.end();
		 ++it)
	{
		EV_SET(&kev, (*it).second->getSocketfd(), EVFILT_READ, EV_ADD, 0, 0, (*it).second);
		kevent(kq, &kev, 1, NULL, 0, NULL);
	}

	struct kevent events[32];
	char buf[1024];

	while (1)
	{
		int n = kevent(kq, NULL, 0, events, sizeof(events) / sizeof(events[0]), NULL);
		for (int i = 0; i < n; i++)
		{
			Socket *socket = (Socket *)events[i].udata;
			if (socket && socket->getType() == LISTEN_ID)
			{
				ListenSocket *listen = dynamic_cast<ListenSocket *>(socket);
				while (1)
				{
					ClientSocket *clientSocket = listen->accept();
					if (!clientSocket)
					{
						if (errno == EAGAIN)
							break;
						perror("accept");
						return -1;
						break;
					}
					EV_SET(&kev, clientSocket->getSocketfd(), EVFILT_READ, EV_ADD, 0, 0, clientSocket);
					kevent(kq, &kev, 1, NULL, 0, NULL);
				}
				continue;
			}
			else if (socket && socket->getType() == CLIENT_ID)
			{
				ClientSocket *client = dynamic_cast<ClientSocket *>(socket);
				/* Client disconnected */
				if (events[i].flags & EV_EOF)
				{
					std::cout << "client [" << client->getHost() << "]: disconnected" << std::endl;
					delete client;
					continue;
				}
				if (events[i].filter == EVFILT_READ)
				{
					while (1)
					{
						client->handleRead();
						ssize_t r = client->recv(buf, sizeof(buf));
						if (r > 0)
						{
							client->send(buf, r);
							write(STDOUT_FILENO, buf, 1);
						}
						else if (r == 0)
						{
							delete client;
							break;
						}
						else
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								write(STDOUT_FILENO, "#", 1);
								break;
							}
							perror("recv");
							delete client;
							break;
						}
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int err = 0;
	if (argc != 2)
	{
		printUsage(argv[0]);
		return 1;
	}

	GlobalConfig config = GlobalConfig();
	try
	{
		Directive::Parser parser = Directive::Parser::openConfigFile(argv[1]);
		Directive::Parser::loadConfigFile(argv[1], config);
		// std::cout << "Configuration parsed successfully" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing configuration: " << e.what() << std::endl;
		return 1;
	}

	std::map<uint64_t, ListenSocket *> sockets;
	std::vector<VirtualServer *> virtualServers;
	ListenSocket *socket = NULL;
	VirtualServer *virtualServer = NULL;

	for (size_t i = 0; i < config.https.size(); i++)
	{
		std::vector<ServerConfig *> servers = config.https[i]->servers;
		for (size_t j = 0; j < servers.size(); j++)
		{
			virtualServer = new VirtualServer(*servers[j]);
			virtualServers.push_back(virtualServer);

			for (size_t l = 0; l < servers[j]->listenAddresses.size(); l++)
			{
				ServerConfig::ListenAddr addr = servers[j]->listenAddresses[l];
				if (sockets.find(addr) == sockets.end())
				{
					socket = new ListenSocket(addr.ip, addr.port);
					if (socket->getSocketfd() < 0)
					{
						delete socket;
						err = 1;
						goto end;
					}
					sockets[(uint64_t)addr] = socket;
				}
				else
				{
					socket = sockets[(uint64_t)addr];
				}
				socket->addVirtualServer(virtualServer);
			}
		}
	}

	event_loop(sockets);

end:
	for (std::map<uint64_t, ListenSocket *>::iterator it = sockets.begin();
		 it != sockets.end();
		 ++it)
	{
		(*it).second->close();
		delete (*it).second;
	}
	for (std::vector<VirtualServer *>::iterator it = virtualServers.begin();
		 it != virtualServers.end();
		 ++it)
	{
		delete (*it);
	}
	virtualServers.clear();
	sockets.clear();
	return (err);
}
