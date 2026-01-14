#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <cstring>

/**
 * @brief Parses the 'autoindex' directive
 * Syntax: autoindex on | off;
 * Example: autoindex on;
 */
struct Config *parseAutoindexDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("autoindex", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("autoindex", "requires exactly one argument (on or off)");

	if (args[1] == "on")
		config->autoindex = true;
	else if (args[1] == "off")
		config->autoindex = false;
	else
		throw Directive::ArgumentException("autoindex", "invalid value '" + args[1] + "' (must be 'on' or 'off')");

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("autoindex", parseAutoindexDirective);
