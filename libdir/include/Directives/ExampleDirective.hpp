#include "Directives.hpp"

#pragma once

class ExampleDirective: public IDirective
{
private:
	
public:
	ExampleDirective();
	ExampleDirective(const std::vector<std::string>& args);
	~ExampleDirective();
	
	DEFINE_ALLOWED_INSIDE();
};
