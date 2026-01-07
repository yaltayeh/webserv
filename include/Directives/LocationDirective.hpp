#include "IDirective.hpp"

#pragma once

class LocationDirective: public IDirective
{
private:
	
public:
	LocationDirective();
	LocationDirective(const std::vector<std::string>& args);
	~LocationDirective();

	DEFINE_ALLOWED_INSIDE();
};
