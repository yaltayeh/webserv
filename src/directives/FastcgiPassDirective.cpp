#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/LocationConfig.hpp"

/**
 * @brief Parses the 'fastcgi_pass' directive
 * Syntax: fastcgi_pass address;
 * Example: fastcgi_pass 127.0.0.1:9000;
 *          fastcgi_pass unix:/var/run/php-fpm.sock;
 */
struct Config *parseFastcgiPassDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	LocationConfig *config = dynamic_cast<LocationConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("fastcgi_pass", "can only be used in location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("fastcgi_pass", "requires exactly one argument");

	// Set value to config
	config->fastcgi_pass = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("fastcgi_pass", parseFastcgiPassDirective);
