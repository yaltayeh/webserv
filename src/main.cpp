#include <iostream>
#include "Config/GlobalConfig.hpp"
#include "libdir/include/Parser.hpp"
#include "EventLoop.hpp"
#include "Parser.hpp"

void printUsage(const char *progName)
{
	std::cout << "Usage: " << progName << " <config-file>" << std::endl;
}

int main(int argc, const char *argv[], const char *envp[])
{
	if (argc != 2)
	{
		printUsage(argv[0]);
		return 1;
	}

	GlobalConfig config = GlobalConfig();
	try
	{
		if (Directive::Parser::loadConfigFile(argv[1], config, envp) == false)
			return (1);

		EventLoop loop(config);
		loop.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
