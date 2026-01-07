#include "RootDirective.hpp"

DEFINE_DIRECTIVE(RootDirective, "root")


RootDirective::RootDirective()
{
}

RootDirective::RootDirective(const std::vector<std::string>& args)
{
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "[" << i << "] " << args[i] << std::endl;
}

RootDirective::~RootDirective()
{
}
