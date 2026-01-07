#include "IDirective.hpp"

#pragma once

class HttpDirective: public IDirective
{
private:
	
public:
	HttpDirective();
	HttpDirective(const std::vector<std::string>& args);
	~HttpDirective();
	DEFINE_ALLOWED_INSIDE();
};
