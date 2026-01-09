#ifndef ERRORPAGEDIRECTIVE_HPP
#define ERRORPAGEDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Config
{
	class ErrorPageDirective : public IDirective
	{
	public:
		const std::vector<int> codes;
		const std::string uri;

	public:
		ErrorPageDirective();
		ErrorPageDirective(const std::vector<int>& codes, const std::string& uri);
		ErrorPageDirective(const ErrorPageDirective& other);
		ErrorPageDirective& operator=(const ErrorPageDirective& other);
		~ErrorPageDirective();

		static ErrorPageDirective* initErrorPageDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
