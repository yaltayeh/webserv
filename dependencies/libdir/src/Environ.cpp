#include "Environ.hpp"
#include <iostream>

Environ::Environ()
{
}

Environ::Environ(const char **envp)
{
	for (size_t i = 0; envp[i]; i++)
	{
		std::string env(envp[i]);
		size_t equl_pos = env.find_first_of('=');
		if (equl_pos == std::string::npos)
			continue;
		std::string name = env.substr(0, equl_pos);
		std::string value = env.substr(equl_pos + 1);
		_table[name] = value;
	}
}

Environ::~Environ()
{
}

const std::string &Environ::getEnv(const std::string &name) const
{
	static std::string empty = "";
	std::map<std::string, std::string>::const_iterator env = _table.find(name);
	if (env == _table.end())
		return (empty);
	return (*env).second;
}
