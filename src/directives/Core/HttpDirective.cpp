#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include "Config/GlobalConfig.hpp"

/**
 * @brief Parses the 'http' directive (block directive)
 * Syntax: http { ... }
 * Example: http {
 *              server { ... }
 *          }
 * Creates the main HTTP configuration context
 */
struct Config *parserHttpDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// HTTP directive takes no arguments
	if (args.size() != 1)
		throw Directive::ArgumentException("http", "takes no arguments");

	// Validate parent context - must be global/root
	GlobalConfig *global = dynamic_cast<GlobalConfig *>(parent);
	if (!global)
		throw Directive::InvalidNesting("http", "can only be used in global context");

	if (global->init_http)
		throw Directive::ConfigException("you can only have one http directive");
	// Create new HTTP config block
	global->http = HttpConfig();
	
	return &global->http;
}

DEFINE_DIRECTIVE("http", parserHttpDirective);
