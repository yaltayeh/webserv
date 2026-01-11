#ifndef CGIPARAMDIRECTIVE_HPP
#define CGIPARAMDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class CgiParamDirective : public IDirective
	{
	public:
		const std::string name;
		const std::string value;

	public:
		CgiParamDirective();
		CgiParamDirective(const std::string& name, const std::string& value);
		CgiParamDirective(const CgiParamDirective& other);
		CgiParamDirective& operator=(const CgiParamDirective& other);
		~CgiParamDirective();

		static CgiParamDirective* initCgiParamDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
