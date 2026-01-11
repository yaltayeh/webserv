#ifndef KEEPALIVETIMEOUTDIRECTIVE_HPP
#define KEEPALIVETIMEOUTDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class KeepaliveTimeoutDirective : public IDirective
	{
	public:
		const std::string timeout;
		const std::string headerTimeout;

	public:
		KeepaliveTimeoutDirective();
		KeepaliveTimeoutDirective(const std::string& timeout, const std::string& headerTimeout);
		KeepaliveTimeoutDirective(const KeepaliveTimeoutDirective& other);
		KeepaliveTimeoutDirective& operator=(const KeepaliveTimeoutDirective& other);
		~KeepaliveTimeoutDirective();

		static KeepaliveTimeoutDirective* initKeepaliveTimeoutDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
