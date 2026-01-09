#include "GlobalDirective.hpp"
#include "IDirective.hpp"
#include "Config.hpp"

Config::GlobalDirective *Config::global;

struct GlobalDirectiveInit
{
	GlobalDirectiveInit()
	{
		Config::global = new Config::GlobalDirective();
	}

	~GlobalDirectiveInit()
	{
		delete Config::global;
	}
} globalDirectiveInit;
