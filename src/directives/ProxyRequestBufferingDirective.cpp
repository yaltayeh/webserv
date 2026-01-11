#include "ProxyRequestBufferingDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: proxy_request_buffering allowed in HttpDirective, ServerDirective, LocationDirective
	DEFINE_DIRECTIVE(ProxyRequestBufferingDirective, "proxy_request_buffering", HttpDirective, ServerDirective, LocationDirective)

	ProxyRequestBufferingDirective* ProxyRequestBufferingDirective::initProxyRequestBufferingDirective(const std::vector<std::string>& args)
	{
		// proxy_request_buffering requires exactly one argument (on or off)
		// Syntax: proxy_request_buffering on|off;
		if (args.size() != 2)
		{
			throw ArgumentException("proxy_request_buffering", "takes exactly one argument (on or off)");
		}

		std::string value = args[1];
		bool enabled;

		if (value == "on")
		{
			enabled = true;
		}
		else if (value == "off")
		{
			enabled = false;
		}
		else
		{
			throw ArgumentException("proxy_request_buffering", "argument must be 'on' or 'off'");
		}

		return new ProxyRequestBufferingDirective(enabled);
	}

	ProxyRequestBufferingDirective::ProxyRequestBufferingDirective()
		: IDirective(), enabled(true)
	{
	}

	ProxyRequestBufferingDirective::ProxyRequestBufferingDirective(bool enabled)
		: IDirective(), enabled(enabled)
	{
	}

	ProxyRequestBufferingDirective::ProxyRequestBufferingDirective(const ProxyRequestBufferingDirective& other)
		: IDirective(other), enabled(other.enabled)
	{
	}

	ProxyRequestBufferingDirective& ProxyRequestBufferingDirective::operator=(const ProxyRequestBufferingDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// const member cannot be reassigned
		}
		return *this;
	}

	ProxyRequestBufferingDirective::~ProxyRequestBufferingDirective()
	{
	}
}
