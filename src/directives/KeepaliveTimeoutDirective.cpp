#include "KeepaliveTimeoutDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: keepalive_timeout allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(KeepaliveTimeoutDirective, "keepalive_timeout", HttpDirective, ServerDirective, LocationDirective)

	KeepaliveTimeoutDirective* KeepaliveTimeoutDirective::initKeepaliveTimeoutDirective(const std::vector<std::string>& args)
	{
		// keepalive_timeout requires at least timeout, optional header_timeout
		if (args.size() < 2 || args.size() > 3)
		{
			throw ArgumentException("keepalive_timeout", "takes one or two arguments (timeout [header_timeout])");
		}

		std::string timeout = args[1];
		std::string headerTimeout = args.size() > 2 ? args[2] : "";
		
		// Validate timeout is not empty
		if (timeout.empty())
		{
			throw ArgumentException("keepalive_timeout", "timeout cannot be empty");
		}

		return new KeepaliveTimeoutDirective(timeout, headerTimeout);
	}

	// Default constructor
	KeepaliveTimeoutDirective::KeepaliveTimeoutDirective()
		: IDirective(), timeout("75s"), headerTimeout("")
	{
	}

	// Parameterized constructor
	KeepaliveTimeoutDirective::KeepaliveTimeoutDirective(const std::string& timeout, const std::string& headerTimeout)
		: IDirective(), timeout(timeout), headerTimeout(headerTimeout)
	{
	}

	// Copy constructor
	KeepaliveTimeoutDirective::KeepaliveTimeoutDirective(const KeepaliveTimeoutDirective& other)
		: IDirective(other), timeout(other.timeout), headerTimeout(other.headerTimeout)
	{
	}

	// Assignment operator
	KeepaliveTimeoutDirective& KeepaliveTimeoutDirective::operator=(const KeepaliveTimeoutDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// timeout and headerTimeout are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	KeepaliveTimeoutDirective::~KeepaliveTimeoutDirective()
	{
	}
}
