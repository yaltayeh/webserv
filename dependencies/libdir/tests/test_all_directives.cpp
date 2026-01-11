#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "ServerNameDirective.hpp"
#include "ListenDirective.hpp"
#include "RootDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "KeepaliveTimeoutDirective.hpp"
#include <iostream>

using namespace Config;

int main()
{
	try
	{
		std::string config = 
			"http {\n"
			"    client_max_body_size 10m;\n"
			"    keepalive_timeout 65s;\n"
			"    root /var/www;\n"
			"    \n"
			"    server {\n"
			"        listen 8080 localhost;\n"
			"        server_name example.com www.example.com;\n"
			"        root /var/www/example;\n"
			"        error_page 404 /404.html;\n"
			"        error_page 500 502 503 504 /50x.html;\n"
			"    }\n"
			"    \n"
			"    server {\n"
			"        listen 443;\n"
			"        server_name secure.example.com;\n"
			"        client_max_body_size 50m;\n"
			"    }\n"
			"}\n";
		
		Parser parser(config);
		
		while (parser.getNextDirectives())
		{
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(Config::global, directive);
		}
		
		// Get http directive
		const HttpDirective* http = Config::global->getFirstChild<HttpDirective>();
		std::cout << "=== HTTP Configuration ===" << std::endl;
		
		// Get http-level directives
		try {
			const ClientMaxBodySizeDirective* httpBodySize = http->getFirstChild<ClientMaxBodySizeDirective>();
			std::cout << "HTTP client_max_body_size: " << httpBodySize->size << std::endl;
		} catch (const DirectiveNotFound&) {}
		
		try {
			const KeepaliveTimeoutDirective* httpKeepalive = http->getFirstChild<KeepaliveTimeoutDirective>();
			std::cout << "HTTP keepalive_timeout: " << httpKeepalive->timeout << std::endl;
		} catch (const DirectiveNotFound&) {}
		
		try {
			const RootDirective* httpRoot = http->getFirstChild<RootDirective>();
			std::cout << "HTTP root: " << httpRoot->path << std::endl;
		} catch (const DirectiveNotFound&) {}
		
		// Get all server directives
		std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
		std::cout << "\nFound " << servers.size() << " server(s)" << std::endl;
		
		for (size_t i = 0; i < servers.size(); i++)
		{
			ServerDirective* server = servers[i];
			std::cout << "\n--- Server " << (i + 1) << " ---" << std::endl;
			
			// Listen
			try {
				const ListenDirective* listen = server->getFirstChild<ListenDirective>();
				std::cout << "  listen: " << listen->host << ":" << listen->port << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Server name
			try {
				const ServerNameDirective* serverName = server->getFirstChild<ServerNameDirective>();
				std::cout << "  server_name:";
				for (size_t j = 0; j < serverName->names.size(); j++)
				{
					std::cout << " " << serverName->names[j];
				}
				std::cout << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Root
			try {
				const RootDirective* root = server->getFirstChild<RootDirective>();
				std::cout << "  root: " << root->path << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Client max body size
			try {
				const ClientMaxBodySizeDirective* bodySize = server->getFirstChild<ClientMaxBodySizeDirective>();
				std::cout << "  client_max_body_size: " << bodySize->size << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Error pages
			std::vector<ErrorPageDirective*> errorPages = server->getChildren<ErrorPageDirective>();
			if (!errorPages.empty())
			{
				std::cout << "  error_page:" << std::endl;
				for (size_t j = 0; j < errorPages.size(); j++)
				{
					std::cout << "    codes:";
					for (size_t k = 0; k < errorPages[j]->codes.size(); k++)
					{
						std::cout << " " << errorPages[j]->codes[k];
					}
					std::cout << " -> " << errorPages[j]->uri << std::endl;
				}
			}
		}
		
		std::cout << "\nAll directives test passed!" << std::endl;
		return 0;
	}
	catch (const ConfigException& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
