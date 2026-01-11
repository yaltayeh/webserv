#include "UploadEnableDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: upload_enable allowed in LocationDirective
	DEFINE_DIRECTIVE(UploadEnableDirective, "upload_enable", LocationDirective)

	UploadEnableDirective* UploadEnableDirective::initUploadEnableDirective(const std::vector<std::string>& args)
	{
		// upload_enable requires exactly one argument: on | off
		if (args.size() != 2)
		{
			throw ArgumentException("upload_enable", "takes exactly one argument (on or off)");
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
			throw ArgumentException("upload_enable", "invalid value, must be 'on' or 'off'");
		}

		return new UploadEnableDirective(enabled);
	}

	// Default constructor
	UploadEnableDirective::UploadEnableDirective()
		: IDirective(), enabled(false)
	{
	}

	// Parameterized constructor
	UploadEnableDirective::UploadEnableDirective(bool enabled)
		: IDirective(), enabled(enabled)
	{
	}

	// Copy constructor
	UploadEnableDirective::UploadEnableDirective(const UploadEnableDirective& other)
		: IDirective(other), enabled(other.enabled)
	{
	}

	// Assignment operator
	UploadEnableDirective& UploadEnableDirective::operator=(const UploadEnableDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// enabled is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	UploadEnableDirective::~UploadEnableDirective()
	{
	}
}
