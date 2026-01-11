#ifndef CGIPASSDIRECTIVE_HPP
#define CGIPASSDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class CgiPassDirective : public IDirective
	{
	public:
		const std::string interpreterPath;

	public:
		CgiPassDirective();
		CgiPassDirective(const std::string& interpreterPath);
		CgiPassDirective(const CgiPassDirective& other);
		CgiPassDirective& operator=(const CgiPassDirective& other);
		~CgiPassDirective();

		static CgiPassDirective* initCgiPassDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
