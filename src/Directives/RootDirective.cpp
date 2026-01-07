#include "Directives/RootDirective.hpp"
#include "Directives/LocationDirective.hpp"

DEFINE_DIRECTIVE(RootDirective, "root", LocationDirective)

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
