#include "IDirective.hpp"

#pragma once

class ServerDirective: public IDirective
{
private:
	
public:
	ServerDirective();
	ServerDirective(const std::vector<std::string>& args);
	~ServerDirective();
};
