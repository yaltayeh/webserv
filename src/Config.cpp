#include "Directives/GlobalDirective.hpp"
#include "IDirective.hpp"
#include "Config.hpp"

GlobalDirective *Config::global;

struct GlobalDirectiveInit
{
	GlobalDirectiveInit()
	{
		Config::global = new GlobalDirective();
	}

	~GlobalDirectiveInit()
	{
		delete Config::global;
	}
} globalDirectiveInit;
