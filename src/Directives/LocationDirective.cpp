#include "Directives/LocationDirective.hpp"
#include "Directives/ServerDirective.hpp"


DEFINE_DIRECTIVE(LocationDirective, "location", ServerDirective)


LocationDirective::LocationDirective()
{
}

LocationDirective::LocationDirective(const std::vector<std::string>& args)
{
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "[" << i << "] " << args[i] << std::endl;
}

LocationDirective::~LocationDirective()
{
}
