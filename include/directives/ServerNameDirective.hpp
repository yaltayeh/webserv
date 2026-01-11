#ifndef SERVERNAMEDIRECTIVE_HPP
#define SERVERNAMEDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Directive
{
	class ServerNameDirective : public IDirective
	{
	public:
		const std::vector<std::string> names;

	public:
		ServerNameDirective();
		ServerNameDirective(const std::vector<std::string>& names);
		ServerNameDirective(const ServerNameDirective& other);
		ServerNameDirective& operator=(const ServerNameDirective& other);
		~ServerNameDirective();

		static ServerNameDirective* initServerNameDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
