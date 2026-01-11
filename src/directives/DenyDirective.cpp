#include "DenyDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "LimitExceptDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: deny allowed in HttpDirective, ServerDirective, LocationDirective, and LimitExceptDirective
	DEFINE_DIRECTIVE(DenyDirective, "deny", HttpDirective, ServerDirective, LocationDirective, LimitExceptDirective)

	DenyDirective* DenyDirective::initDenyDirective(const std::vector<std::string>& args)
	{
		// deny requires exactly one argument (address)
		if (args.size() != 2)
		{
			throw ArgumentException("deny", "takes exactly one argument (address, CIDR, or 'all')");
		}

		std::string address = args[1];
		
		// Validate address is not empty
		if (address.empty())
		{
			throw ArgumentException("deny", "address cannot be empty");
		}

		// Basic validation for "all" or IP-like format
		// Note: Full IP/CIDR validation can be done at runtime in webserv
		if (address != "all" && address.find_first_not_of("0123456789./") != std::string::npos)
		{
			// Contains characters other than digits, dots, and slashes
			// Could still be valid (e.g., IPv6), so just accept it
		}

		return new DenyDirective(address);
	}

	// Default constructor
	DenyDirective::DenyDirective()
		: IDirective(), address("all")
	{
	}

	// Parameterized constructor
	DenyDirective::DenyDirective(const std::string& address)
		: IDirective(), address(address)
	{
	}

	// Copy constructor
	DenyDirective::DenyDirective(const DenyDirective& other)
		: IDirective(other), address(other.address)
	{
	}

	// Assignment operator
	DenyDirective& DenyDirective::operator=(const DenyDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// address is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	DenyDirective::~DenyDirective()
	{
	}
}
