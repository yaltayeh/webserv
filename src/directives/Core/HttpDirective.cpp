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
struct Config *parserHttpDirective(const std::vector<std::string> &args, Config *parent)
{
	// HTTP directive takes no arguments
	if (args.size() != 1)
		throw Directive::ArgumentException("http", "takes no arguments");

	// Validate parent context - must be global/root
	GlobalConfig *global = dynamic_cast<GlobalConfig *>(parent);
	if (!global)
		throw Directive::InvalidNesting("http", "can only be used in global context");

	// Create new HTTP config block
	HttpConfig http = HttpConfig();
	global->https.push_back(http);
	
	return &global->https[global->https.size() - 1];
}

DEFINE_DIRECTIVE("http", parserHttpDirective);
