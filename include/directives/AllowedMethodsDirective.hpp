#ifndef ALLOWEDMETHODSDIRECTIVE_HPP
#define ALLOWEDMETHODSDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Directive
{
	class AllowedMethodsDirective : public IDirective
	{
	public:
		const std::vector<std::string> methods;

	public:
		AllowedMethodsDirective();
		AllowedMethodsDirective(const std::vector<std::string>& methods);
		AllowedMethodsDirective(const AllowedMethodsDirective& other);
		AllowedMethodsDirective& operator=(const AllowedMethodsDirective& other);
		~AllowedMethodsDirective();

		static AllowedMethodsDirective* initAllowedMethodsDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
