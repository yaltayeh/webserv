#include <iostream>
#include "Config/GlobalConfig.hpp"
#include "libdir/include/Parser.hpp"
#include "EventLoop.hpp"
#include "Parser.hpp"

int main(int argc, const char *argv[], const char *envp[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <config-file>" << std::endl;
		return 1;
	}

	GlobalConfig config = GlobalConfig();
	try
	{
		Directive::Parser::loadConfigFile(argv[1], config, envp);

		EventLoop loop(config);
		loop.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 2;
	}
	return 0;
}
