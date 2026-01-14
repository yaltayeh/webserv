#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'fastcgi_index' directive
 * Syntax: fastcgi_index name;
 * Example: fastcgi_index index.php;
 */
struct Config *parseFastcgiIndexDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("fastcgi_index", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("fastcgi_index", "requires exactly one argument");

	// Set value to config
	config->fastcgi_index = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("fastcgi_index", parseFastcgiIndexDirective);
