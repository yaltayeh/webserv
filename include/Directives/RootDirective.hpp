#include "Directives.hpp"

#pragma once

class RootDirective : public IDirective
{
private:
	std::string path;

public:
	RootDirective();
	RootDirective(const std::vector<std::string> &args);
	~RootDirective();
	const std::string &getPath() const;
	DEFINE_ALLOWED_INSIDE();
};
