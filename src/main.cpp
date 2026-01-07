// #include "Data.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Tree.hpp"
#include "Parser.hpp"
#include "Directives/ServerDirective.hpp"
#include "Directives/HttpDirective.hpp"
#include "Directives/LocationDirective.hpp"
#include "Directives/RootDirective.hpp"
#include <typeinfo>
#include "Config.hpp"

int readConfigFile(const char *fileName, std::string &content)
{
	std::ifstream file(fileName);
	if (!file)
	{
		std::cerr << "Cannot open file.\n";
		return 1;
	}

	std::stringstream ss;
	ss << file.rdbuf();

	content = ss.str();
	return (0);
}

int main()
{
	std::string conf;
	if (readConfigFile("example.conf", conf) != 0)
		return (1);

	Config::Parser parser(conf);

	parser.getNextDirectives();
	std::string directive = parser.copyDirective();
	IDirective::parseDirective(Config::global, directive);
	return (0);
}