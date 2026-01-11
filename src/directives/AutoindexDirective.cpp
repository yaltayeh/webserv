#include "AutoindexDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: autoindex allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(AutoindexDirective, "autoindex", HttpDirective, ServerDirective, LocationDirective)

	AutoindexDirective* AutoindexDirective::initAutoindexDirective(const std::vector<std::string>& args)
	{
		// autoindex requires exactly one argument: on | off
		if (args.size() != 2)
		{
			throw ArgumentException("autoindex", "takes exactly one argument (on or off)");
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
			throw ArgumentException("autoindex", "invalid value, must be 'on' or 'off'");
		}

		return new AutoindexDirective(enabled);
	}

	// Default constructor
	AutoindexDirective::AutoindexDirective()
		: IDirective(), enabled(false)
	{
	}

	// Parameterized constructor
	AutoindexDirective::AutoindexDirective(bool enabled)
		: IDirective(), enabled(enabled)
	{
	}

	// Copy constructor
	AutoindexDirective::AutoindexDirective(const AutoindexDirective& other)
		: IDirective(other), enabled(other.enabled)
	{
	}

	// Assignment operator
	AutoindexDirective& AutoindexDirective::operator=(const AutoindexDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// enabled is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	AutoindexDirective::~AutoindexDirective()
	{
	}
}
