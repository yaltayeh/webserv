#include "AutoindexFormatDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: autoindex_format allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(AutoindexFormatDirective, "autoindex_format", HttpDirective, ServerDirective, LocationDirective)

	AutoindexFormatDirective* AutoindexFormatDirective::initAutoindexFormatDirective(const std::vector<std::string>& args)
	{
		// autoindex_format requires exactly one argument: html | xml | json | jsonp
		if (args.size() != 2)
		{
			throw ArgumentException("autoindex_format", "takes exactly one argument (html, xml, json, or jsonp)");
		}

		std::string format = args[1];
		
		// Validate format value
		if (format != "html" && format != "xml" && format != "json" && format != "jsonp")
		{
			throw ArgumentException("autoindex_format", "invalid format, must be 'html', 'xml', 'json', or 'jsonp'");
		}

		return new AutoindexFormatDirective(format);
	}

	// Default constructor
	AutoindexFormatDirective::AutoindexFormatDirective()
		: IDirective(), format("html")
	{
	}

	// Parameterized constructor
	AutoindexFormatDirective::AutoindexFormatDirective(const std::string& format)
		: IDirective(), format(format)
	{
	}

	// Copy constructor
	AutoindexFormatDirective::AutoindexFormatDirective(const AutoindexFormatDirective& other)
		: IDirective(other), format(other.format)
	{
	}

	// Assignment operator
	AutoindexFormatDirective& AutoindexFormatDirective::operator=(const AutoindexFormatDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// format is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	AutoindexFormatDirective::~AutoindexFormatDirective()
	{
	}
}
