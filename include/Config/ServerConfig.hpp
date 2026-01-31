#pragma once

#include <vector>
#include <string>
#include <map>
#include "CommonConfig.hpp"
#include "LocationConfig.hpp"
#include <arpa/inet.h>

struct ServerConfig : public CommonConfigServerLocation
{
	// Listen directives: address:port pairs
	struct ListenAddr
	{
		uint32_t ip;   // network byte order
		uint16_t port; // host byte order
		operator uint64_t()
		{
			return ((uint64_t)ip << 16) | port;
		}
	};
	std::vector<ListenAddr> listenAddresses;
	bool is_new_listen;

	// Server names (virtual host)
	std::vector<std::string> server_names;

	ServerConfig()
		: CommonConfigServerLocation(),
		  listenAddresses(),
		  is_new_listen(false),
		  server_names()
	{
		uint32_t ip = htonl(INADDR_ANY);
		struct ListenAddr addr8080 = {ip, 8080};
		listenAddresses.push_back(addr8080);
	};

	ServerConfig(const ServerConfig &other)
		: CommonConfigServerLocation(other)
	{
		this->listenAddresses = other.listenAddresses;
		this->is_new_listen = other.is_new_listen;
		this->server_names = other.server_names;
	};

	ServerConfig& operator=(const ServerConfig &other)
	{
		if (this != &other)
		{
			CommonConfigServerLocation::operator=(other);
			this->listenAddresses = other.listenAddresses;
			this->is_new_listen = other.is_new_listen;
			this->server_names = other.server_names;
		}
		return *this;
	}

	~ServerConfig() {}
};
