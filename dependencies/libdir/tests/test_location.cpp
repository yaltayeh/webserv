#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "ServerNameDirective.hpp"
#include "ListenDirective.hpp"
#include "LocationDirective.hpp"
#include "RootDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include <iostream>

using namespace Config;

int main()
{
	try
	{
		std::string config = 
			"http {\n"
			"    server {\n"
			"        listen 8080;\n"
			"        server_name example.com;\n"
			"        root /var/www;\n"
			"        \n"
			"        location / {\n"
			"            root /var/www/html;\n"
			"        }\n"
			"        \n"
			"        location = /exact {\n"
			"            root /var/www/exact;\n"
			"        }\n"
			"        \n"
			"        location /static/ {\n"
			"            root /var/www/static;\n"
			"        }\n"
			"        \n"
			"        location @named {\n"
			"            root /var/www/named;\n"
			"        }\n"
			"        \n"
			"        location /api/ {\n"
			"            client_max_body_size 50m;\n"
			"            error_page 404 /api/404.json;\n"
			"            \n"
			"            location /api/v1/ {\n"
			"                root /var/www/api/v1;\n"
			"            }\n"
			"        }\n"
			"    }\n"
			"}\n";
		
		Parser parser(config);
		
		while (parser.getNextDirectives())
		{
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(Config::global, directive);
		}
		
		// Get http -> server
		const HttpDirective* http = Config::global->getFirstChild<HttpDirective>();
		const ServerDirective* server = http->getFirstChild<ServerDirective>();
		
		std::cout << "=== Location Directive Test ===" << std::endl;
		
		const ListenDirective* listen = server->getFirstChild<ListenDirective>();
		const ServerNameDirective* serverName = server->getFirstChild<ServerNameDirective>();
		std::cout << "Server: " << serverName->names[0] << " on port " << listen->port << std::endl;
		
		// Get all locations
		std::vector<LocationDirective*> locations = server->getChildren<LocationDirective>();
		std::cout << "\nFound " << locations.size() << " location(s):" << std::endl;
		
		for (size_t i = 0; i < locations.size(); i++)
		{
			LocationDirective* loc = locations[i];
			std::cout << "\n--- Location " << (i + 1) << " ---" << std::endl;
			
			if (loc->modifier.empty())
			{
				std::cout << "  Pattern: " << loc->uri << " (prefix match)" << std::endl;
			}
			else
			{
				std::cout << "  Pattern: " << loc->modifier << " " << loc->uri;
				
				if (loc->modifier == "=")
					std::cout << " (exact match)";
				else if (loc->modifier[0] == '@')
					std::cout << " (named location)";
				
				std::cout << std::endl;
			}
			
			// Check for root directive
			try {
				const RootDirective* root = loc->getFirstChild<RootDirective>();
				std::cout << "  root: " << root->path << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Check for client_max_body_size
			try {
				const ClientMaxBodySizeDirective* bodySize = loc->getFirstChild<ClientMaxBodySizeDirective>();
				std::cout << "  client_max_body_size: " << bodySize->size << std::endl;
			} catch (const DirectiveNotFound&) {}
			
			// Check for error_page
			std::vector<ErrorPageDirective*> errorPages = loc->getChildren<ErrorPageDirective>();
			for (size_t j = 0; j < errorPages.size(); j++)
			{
				std::cout << "  error_page:";
				for (size_t k = 0; k < errorPages[j]->codes.size(); k++)
				{
					std::cout << " " << errorPages[j]->codes[k];
				}
				std::cout << " -> " << errorPages[j]->uri << std::endl;
			}
			
			// Check for nested locations
			std::vector<LocationDirective*> nestedLocs = loc->getChildren<LocationDirective>();
			if (!nestedLocs.empty())
			{
				std::cout << "  Nested locations: " << nestedLocs.size() << std::endl;
				for (size_t j = 0; j < nestedLocs.size(); j++)
				{
					std::cout << "    - " << nestedLocs[j]->modifier << " " << nestedLocs[j]->uri << std::endl;
					
					try {
						const RootDirective* nestedRoot = nestedLocs[j]->getFirstChild<RootDirective>();
						std::cout << "      root: " << nestedRoot->path << std::endl;
					} catch (const DirectiveNotFound&) {}
				}
			}
		}
		
		std::cout << "\n=== All tests passed! ===" << std::endl;
		return 0;
	}
	catch (const ConfigException& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
