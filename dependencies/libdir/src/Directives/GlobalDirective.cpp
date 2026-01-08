#include "Directives/GlobalDirective.hpp"
#include "GlobalDirective.hpp"

GlobalDirective::GlobalDirective() {}

GlobalDirective::~GlobalDirective() {}

std::string GlobalDirective::getName() const
{
	return std::string("global");
}

const std::type_info **GlobalDirective::getAllowedInside() const
{
	static const std::type_info **allowedInside = {
		NULL};

	return allowedInside;
}
