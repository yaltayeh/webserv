#pragma once

#include <string>
#include <map>

class Environ
{
private:
	std::map<std::string, std::string> _table;

public:
	Environ();
	Environ(const char **envp);
	~Environ();

	const std::string &getEnv(const std::string &name) const;
};
