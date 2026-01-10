#ifndef AUTOINDEXEXACTSIZEDIRECTIVE_HPP
#define AUTOINDEXEXACTSIZEDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class AutoindexExactSizeDirective : public IDirective
	{
	public:
		const bool enabled;

	public:
		AutoindexExactSizeDirective();
		AutoindexExactSizeDirective(bool enabled);
		AutoindexExactSizeDirective(const AutoindexExactSizeDirective& other);
		AutoindexExactSizeDirective& operator=(const AutoindexExactSizeDirective& other);
		~AutoindexExactSizeDirective();

		static AutoindexExactSizeDirective* initAutoindexExactSizeDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
