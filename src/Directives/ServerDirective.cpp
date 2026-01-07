#include "Directives/ServerDirective.hpp"
#include "Directives/HttpDirective.hpp"
#include "Directives/GlobalDirective.hpp"

DEFINE_DIRECTIVE(ServerDirective, "server", GlobalDirective, HttpDirective)


ServerDirective::ServerDirective()
{
}

ServerDirective::ServerDirective(const std::vector<std::string>& args)
{
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "[" << i << "] " << args[i] << std::endl;
}

ServerDirective::~ServerDirective()
{
}
