#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "ServerNameDirective.hpp"
#include <iostream>

using namespace Config;

int main()
{
	try
	{
		std::string config = 
			"http {\n"
			"    server {\n"
			"        server_name example.com www.example.com;\n"
			"    }\n"
			"    server {\n"
			"        server_name api.example.com;\n"
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
		std::cout << "Found http block" << std::endl;
		
		// Get all server directives
		std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
		std::cout << "Found " << servers.size() << " server block(s)" << std::endl;
		
		for (size_t i = 0; i < servers.size(); i++)
		{
			ServerDirective* server = servers[i];
			std::cout << "\nServer " << i << ":" << std::endl;
			
			// Get server_name directive
			const ServerNameDirective* serverName = server->getFirstChild<ServerNameDirective>();
			std::cout << "  server_name:";
			for (size_t j = 0; j < serverName->names.size(); j++)
			{
				std::cout << " " << serverName->names[j];
			}
			std::cout << std::endl;
		}
		
		std::cout << "\nTest passed!" << std::endl;
		return 0;
	}
	catch (const ConfigException& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
