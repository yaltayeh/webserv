#include "IDirective.hpp"

#pragma once

class RootDirective: public IDirective
{
private:
	
public:
	RootDirective();
	RootDirective(const std::vector<std::string>& args);
	~RootDirective();
};
