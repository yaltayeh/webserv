#ifndef CGIEXTENSIONDIRECTIVE_HPP
#define CGIEXTENSIONDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Directive
{
	class CgiExtensionDirective : public IDirective
	{
	public:
		const std::vector<std::string> extensions;

	public:
		CgiExtensionDirective();
		CgiExtensionDirective(const std::vector<std::string>& extensions);
		CgiExtensionDirective(const CgiExtensionDirective& other);
		CgiExtensionDirective& operator=(const CgiExtensionDirective& other);
		~CgiExtensionDirective();

		static CgiExtensionDirective* initCgiExtensionDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
