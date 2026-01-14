#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'deny' directive
 * Syntax: deny all;
 * Example: deny all;
 */
struct Config *parseDenyDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("deny", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("deny", "requires exactly one argument");

	if (args[1] == "all")
		config->deny_all = true;
	else
		throw Directive::ArgumentException("deny", "only 'all' is supported currently");

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("deny", parseDenyDirective);
