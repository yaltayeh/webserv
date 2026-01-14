#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'autoindex_format' directive
 * Syntax: autoindex_format html | xml | json | jsonp;
 * Example: autoindex_format html;
 *          autoindex_format json;
 */
struct Config *parseAutoindexFormatDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("autoindex_format", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("autoindex_format", "requires exactly one argument");

	std::string format = args[1];
	if (format != "html" && format != "xml" && format != "json" && format != "jsonp")
		throw Directive::ArgumentException("autoindex_format", "invalid format '" + format + "' (must be html, xml, json, or jsonp)");

	// Set value to config
	config->autoindex_format = format;

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("autoindex_format", parseAutoindexFormatDirective);
