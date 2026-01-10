#ifndef AUTOINDEXFORMATDIRECTIVE_HPP
#define AUTOINDEXFORMATDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class AutoindexFormatDirective : public IDirective
	{
	public:
		const std::string format;

	public:
		AutoindexFormatDirective();
		AutoindexFormatDirective(const std::string& format);
		AutoindexFormatDirective(const AutoindexFormatDirective& other);
		AutoindexFormatDirective& operator=(const AutoindexFormatDirective& other);
		~AutoindexFormatDirective();

		static AutoindexFormatDirective* initAutoindexFormatDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
