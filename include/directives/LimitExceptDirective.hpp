#ifndef LIMITEXCEPTDIRECTIVE_HPP
#define LIMITEXCEPTDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Directive
{
	class LimitExceptDirective : public IDirective
	{
	public:
		const std::vector<std::string> methods;  // Allowed HTTP methods (all others are denied)

	public:
		LimitExceptDirective();
		LimitExceptDirective(const std::vector<std::string>& methods);
		LimitExceptDirective(const LimitExceptDirective& other);
		LimitExceptDirective& operator=(const LimitExceptDirective& other);
		~LimitExceptDirective();

		static LimitExceptDirective* initLimitExceptDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
