#include <iostream>
#include "Config/GlobalConfig.hpp"
#include "libdir/include/Parser.hpp"
#include "EventLoop.hpp"
#include "Parser.hpp"

void printUsage(const char *progName)
{
	std::cout << "Usage: " << progName << " <config-file>" << std::endl;
}

int main(int argc, char *argv[])
{
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

	EventLoop loop(config);
	if (loop.init() != 0)
		return (1);
	if (loop.run() != 0)
		return (1);
	return (0);
}
