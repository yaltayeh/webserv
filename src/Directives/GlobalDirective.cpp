#include "GlobalDirective.hpp"

GlobalDirective::GlobalDirective()
{
}

GlobalDirective::~GlobalDirective()
{
}

const std::type_info **GlobalDirective::getAllowedInside() const {
	static const std::type_info **allowedInside = {
		NULL
	};

	return allowedInside;
}
