#include "IDirective.hpp"

#pragma once

class GlobalDirective: public IDirective
{
private:
public:
	GlobalDirective();
	~GlobalDirective();

	DEFINE_ALLOWED_INSIDE();
};