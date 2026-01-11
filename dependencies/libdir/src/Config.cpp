#include "GlobalDirective.hpp"
#include "IDirective.hpp"
#include "Config.hpp"

Directive::GlobalDirective *Directive::global;

struct GlobalDirectiveInit
{
	GlobalDirectiveInit()
	{
		Directive::global = new Directive::GlobalDirective();
	}

	~GlobalDirectiveInit()
	{
		delete Directive::global;
	}
} globalDirectiveInit;
