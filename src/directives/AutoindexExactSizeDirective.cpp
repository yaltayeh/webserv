#include "AutoindexExactSizeDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: autoindex_exact_size allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(AutoindexExactSizeDirective, "autoindex_exact_size", HttpDirective, ServerDirective, LocationDirective)

	AutoindexExactSizeDirective* AutoindexExactSizeDirective::initAutoindexExactSizeDirective(const std::vector<std::string>& args)
	{
		// autoindex_exact_size requires exactly one argument: on | off
		if (args.size() != 2)
		{
			throw ArgumentException("autoindex_exact_size", "takes exactly one argument (on or off)");
		}

		std::string value = args[1];
		bool enabled = false;
		
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
			throw ArgumentException("autoindex_exact_size", "invalid value, must be 'on' or 'off'");
		}

		return new AutoindexExactSizeDirective(enabled);
	}

	// Default constructor
	AutoindexExactSizeDirective::AutoindexExactSizeDirective()
		: IDirective(), enabled(true)
	{
	}

	// Parameterized constructor
	AutoindexExactSizeDirective::AutoindexExactSizeDirective(bool enabled)
		: IDirective(), enabled(enabled)
	{
	}

	// Copy constructor
	AutoindexExactSizeDirective::AutoindexExactSizeDirective(const AutoindexExactSizeDirective& other)
		: IDirective(other), enabled(other.enabled)
	{
	}

	// Assignment operator
	AutoindexExactSizeDirective& AutoindexExactSizeDirective::operator=(const AutoindexExactSizeDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// enabled is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	AutoindexExactSizeDirective::~AutoindexExactSizeDirective()
	{
	}
}
