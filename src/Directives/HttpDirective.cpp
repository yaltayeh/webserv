#include "HttpDirective.hpp"

DEFINE_DIRECTIVE(HttpDirective, "http")


HttpDirective::HttpDirective()
{
}

HttpDirective::HttpDirective(const std::vector<std::string>& args)
{
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "[" << i << "] " << args[i] << std::endl;
}

HttpDirective::~HttpDirective()
{
}
