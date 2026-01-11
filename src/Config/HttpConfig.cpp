#include "../include/Config/HttpConfig.hpp"
#include "../include/Config/ServerConfig.hpp"
#include "directives/HttpDirective.hpp"
#include "directives/ServerDirective.hpp"
#include "Exceptions.hpp"

HttpConfig::HttpConfig()
	: servers()
{
}

HttpConfig::HttpConfig(const HttpConfig& other)
	: servers(other.servers)
{
}

HttpConfig& HttpConfig::operator=(const HttpConfig& other)
{
	if (this != &other)
	{
		servers = other.servers;
	}
	return *this;
}

HttpConfig::~HttpConfig()
{
}

HttpConfig HttpConfig::fromDirective(const Directive::HttpDirective* directive)
{
	HttpConfig config;
	
	// Extract all server directives
	try {
		Directive::ListDirectives<Directive::ServerDirective> serverDirectives = 
			directive->getMany<Directive::ServerDirective>();
		
		for (size_t i = 0; i < serverDirectives.size(); ++i)
		{
			ServerConfig serverConfig = ServerConfig::fromDirective(serverDirectives[i]);
			config.servers.push_back(serverConfig);
		}
	} catch (const Directive::DirectiveNotFound&) {
		// No servers defined
	}
	
	return config;
}
