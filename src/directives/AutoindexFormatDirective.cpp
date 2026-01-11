#include "AutoindexFormatDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: autoindex_format allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(AutoindexFormatDirective, "autoindex_format", HttpDirective, ServerDirective, LocationDirective)

	AutoindexFormatDirective *AutoindexFormatDirective::initAutoindexFormatDirective(const std::vector<std::string> &args)
	{
		// autoindex_format requires exactly one argument: html | xml | json | jsonp
		if (args.size() != 2)
		{
			throw ArgumentException("autoindex_format", "takes exactly one argument (html, xml, json, or jsonp)");
		}

		std::string format_str = args[1];
		enum auto_index_format format;

		if (format_str == "html")
			format = HTTP_AUTOINDEX_HTML;
		else if (format_str == "json")
			format = HTTP_AUTOINDEX_JSON;
		else if (format_str == "jsonp")
			format = HTTP_AUTOINDEX_JSONP;
		else if (format_str == "xml")
			format = HTTP_AUTOINDEX_XML;
		else 
			throw ArgumentException("autoindex_format", "invalid format, must be 'html', 'xml', 'json', or 'jsonp'");

		return new AutoindexFormatDirective(format);
	}

	// Default constructor
	AutoindexFormatDirective::AutoindexFormatDirective()
		: IDirective(), format(HTTP_AUTOINDEX_HTML)
	{
	}

	// Parameterized constructor
	AutoindexFormatDirective::AutoindexFormatDirective(auto_index_format format)
		: IDirective(), format(format)
	{
	}

	// Copy constructor
	AutoindexFormatDirective::AutoindexFormatDirective(const AutoindexFormatDirective &other)
		: IDirective(other), format(other.format)
	{
	}

	// Assignment operator
	AutoindexFormatDirective &AutoindexFormatDirective::operator=(const AutoindexFormatDirective &other)
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
