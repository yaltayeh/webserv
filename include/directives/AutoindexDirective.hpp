#ifndef AUTOINDEXDIRECTIVE_HPP
#define AUTOINDEXDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class AutoindexDirective : public IDirective
	{
	public:
		const bool enabled;

	public:
		AutoindexDirective();
		AutoindexDirective(bool enabled);
		AutoindexDirective(const AutoindexDirective& other);
		AutoindexDirective& operator=(const AutoindexDirective& other);
		~AutoindexDirective();

		static AutoindexDirective* initAutoindexDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
