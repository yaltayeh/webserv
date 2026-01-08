// #include "Data.hpp"
#include <iostream>
#include <sstream>
#include "Tree.hpp"
#include "Parser.hpp"
#include "Directives/ServerDirective.hpp"
#include "Directives/HttpDirective.hpp"
#include "Directives/LocationDirective.hpp"
#include "Directives/RootDirective.hpp"
#include <typeinfo>
#include "Config.hpp"

int main()
{

	try
	{
		Config::Parser parser = Config::Parser::openConfigFile("example.conf");
		while (parser.getNextDirectives())
		{
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(Config::global, directive);
		}

		ListDirectives<ServerDirective> servers = Config::global->getMany<ServerDirective>();
		ListDirectives<LocationDirective> locations = servers[0]->getMany<LocationDirective>();
		locations[0]->getFirstChild<RootDirective>();
		const RootDirective *root = locations[0]->getOne<RootDirective>();
		std::cout << "Path: " << root->getPath() << std::endl;

		return (0);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}