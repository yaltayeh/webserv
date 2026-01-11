#ifndef HTTPDIRECTIVE_HPP
#define HTTPDIRECTIVE_HPP

#include "IDirective.hpp"

namespace Directive
{
	class HttpDirective : public IDirective
	{
	public:
		HttpDirective();
		HttpDirective(const HttpDirective& other);
		HttpDirective& operator=(const HttpDirective& other);
		~HttpDirective();

		static HttpDirective* initHttpDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
