#include "AutoindexLocaltimeDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: autoindex_localtime allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(AutoindexLocaltimeDirective, "autoindex_localtime", HttpDirective, ServerDirective, LocationDirective)

	AutoindexLocaltimeDirective* AutoindexLocaltimeDirective::initAutoindexLocaltimeDirective(const std::vector<std::string>& args)
	{
		// autoindex_localtime requires exactly one argument: on | off
		if (args.size() != 2)
		{
			throw ArgumentException("autoindex_localtime", "takes exactly one argument (on or off)");
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
			throw ArgumentException("autoindex_localtime", "invalid value, must be 'on' or 'off'");
		}

		return new AutoindexLocaltimeDirective(enabled);
	}

	// Default constructor
	AutoindexLocaltimeDirective::AutoindexLocaltimeDirective()
		: IDirective(), enabled(false)
	{
	}

	// Parameterized constructor
	AutoindexLocaltimeDirective::AutoindexLocaltimeDirective(bool enabled)
		: IDirective(), enabled(enabled)
	{
	}

	// Copy constructor
	AutoindexLocaltimeDirective::AutoindexLocaltimeDirective(const AutoindexLocaltimeDirective& other)
		: IDirective(other), enabled(other.enabled)
	{
	}

	// Assignment operator
	AutoindexLocaltimeDirective& AutoindexLocaltimeDirective::operator=(const AutoindexLocaltimeDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// enabled is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	AutoindexLocaltimeDirective::~AutoindexLocaltimeDirective()
	{
	}
}
