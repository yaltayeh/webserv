#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/LocationConfig.hpp"
#include "libdir/include/Parser.hpp"

/**
 * @brief Parses the 'alias' directive
 * Syntax: alias path;
 * Example: alias /var/www/files/;
 * Note: Replaces location path in URI, unlike root which appends
 */
struct Config *parseAliasDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	LocationConfig *config = dynamic_cast<LocationConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("alias", "can only be used in location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("alias", "requires exactly one argument");

	// Set value to config
	config->alias = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("alias", parseAliasDirective);
