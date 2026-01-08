#include "Directives/RootDirective.hpp"
#include "Directives/LocationDirective.hpp"

DEFINE_DIRECTIVE(RootDirective, "root", LocationDirective)

RootDirective::RootDirective()
{
	
}

RootDirective::RootDirective(const std::vector<std::string>& args)
{
	// for (size_t i = 0; i < args.size(); i++)
	// 	std::cout << "[" << i << "] " << args[i] << std::endl;
	if (args.size() != 2)
		throw std::runtime_error("RootDirective: args.size() != 1");// TODO
	path = args[1];
}

RootDirective::~RootDirective()
{
}

const std::string &RootDirective::getPath() const
{
	return path;
}
