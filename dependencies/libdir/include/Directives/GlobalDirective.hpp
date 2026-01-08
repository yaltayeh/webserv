#include "Directives.hpp"

#pragma once

class GlobalDirective: public IDirective
{
private:
public:
	GlobalDirective();
	~GlobalDirective();

	std::string getName() const;

	DEFINE_ALLOWED_INSIDE();
};