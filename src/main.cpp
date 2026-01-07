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

		std::cout << "global children count : " << Config::global->getChildren<ServerDirective>("server").size() << std::endl;
		return (0);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}