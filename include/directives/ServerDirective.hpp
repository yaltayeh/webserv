#ifndef SERVERDIRECTIVE_HPP
#define SERVERDIRECTIVE_HPP

#include "IDirective.hpp"

namespace Directive
{
	class ServerDirective : public IDirective
	{
	public:
		ServerDirective();
		ServerDirective(const ServerDirective& other);
		ServerDirective& operator=(const ServerDirective& other);
		~ServerDirective();

		static ServerDirective* initServerDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
