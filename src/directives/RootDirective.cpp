#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'root' directive
 * Syntax: root path;
 * Example: root /var/www/html;
 *          root /tmp/www;
 */
struct Config *parseRootDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("root", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("root", "requires exactly one argument");

	// Set value to config
	config->root = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("root", parseRootDirective);
