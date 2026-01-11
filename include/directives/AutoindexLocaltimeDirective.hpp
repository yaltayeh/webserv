#ifndef AUTOINDEXLOCALTIMEDIRECTIVE_HPP
#define AUTOINDEXLOCALTIMEDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class AutoindexLocaltimeDirective : public IDirective
	{
	public:
		const bool enabled;

	public:
		AutoindexLocaltimeDirective();
		AutoindexLocaltimeDirective(bool enabled);
		AutoindexLocaltimeDirective(const AutoindexLocaltimeDirective& other);
		AutoindexLocaltimeDirective& operator=(const AutoindexLocaltimeDirective& other);
		~AutoindexLocaltimeDirective();

		static AutoindexLocaltimeDirective* initAutoindexLocaltimeDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
