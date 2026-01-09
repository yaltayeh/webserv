#ifndef PROXYREQUESTBUFFERINGDIRECTIVE_HPP
#define PROXYREQUESTBUFFERINGDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class ProxyRequestBufferingDirective : public IDirective
	{
	public:
		const bool enabled;

	public:
		ProxyRequestBufferingDirective();
		ProxyRequestBufferingDirective(bool enabled);
		ProxyRequestBufferingDirective(const ProxyRequestBufferingDirective& other);
		ProxyRequestBufferingDirective& operator=(const ProxyRequestBufferingDirective& other);
		~ProxyRequestBufferingDirective();

		static ProxyRequestBufferingDirective* initProxyRequestBufferingDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
