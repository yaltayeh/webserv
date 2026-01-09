#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "GlobalDirective.hpp"
#include "Exceptions.hpp"
#include "../directives/include/ServerNameDirective.hpp"
#include <iostream>

using namespace Config;

int main()
{
	try
	{
		std::string config = 
			"server_name example.com www.example.com;\n"
			"server_name api.example.com;\n";
		
		Parser parser(config);
		
		while (parser.getNextDirectives())
		{
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(Config::global, directive);
		}
		
		// Get all server_name directives
		std::vector<ServerNameDirective*> serverNames = 
			Config::global->getChildren<ServerNameDirective>();
		
		std::cout << "Found " << serverNames.size() << " server_name directive(s):" << std::endl;
		
		for (size_t i = 0; i < serverNames.size(); i++)
		{
			ServerNameDirective* sn = serverNames[i];
			std::cout << "  server_name " << i << " has " << sn->names.size() << " domain(s):" << std::endl;
			
			for (size_t j = 0; j < sn->names.size(); j++)
			{
				std::cout << "    - " << sn->names[j] << std::endl;
			}
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
