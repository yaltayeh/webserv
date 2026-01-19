#pragma once

#include <string>

class RegEx
{
private:

	std::string	_pattern;

public:

	RegEx(const std::string &pattern);
	~RegEx();

	int     		match(const std::string &str) const;
	static int  	matchPattern(const std::string &pattern,
							const std::string &str);

};
