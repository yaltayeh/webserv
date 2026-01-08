#include "Directives/ExampleDirective.hpp"
#include "Directives/GlobalDirective.hpp"

DEFINE_DIRECTIVE(ExampleDirective, "example", GlobalDirective)

ExampleDirective::ExampleDirective() {}

ExampleDirective::ExampleDirective(const std::vector<std::string> &args)
{
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "[" << i << "] " << args[i] << std::endl;
}

ExampleDirective::~ExampleDirective() {}
