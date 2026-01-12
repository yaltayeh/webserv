#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'autoindex_exact_size' directive
 * Syntax: autoindex_exact_size on | off;
 * Example: autoindex_exact_size off;
 * Controls whether file sizes are shown exactly or in human-readable format
 */
struct Config *parseAutoindexExactSizeDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("autoindex_exact_size", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("autoindex_exact_size", "requires exactly one argument (on or off)");

	if (args[1] == "on")
		config->autoindex_exact_size = true;
	else if (args[1] == "off")
		config->autoindex_exact_size = false;
	else
		throw Directive::ArgumentException("autoindex_exact_size", "invalid value '" + args[1] + "' (must be 'on' or 'off')");

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("autoindex_exact_size", parseAutoindexExactSizeDirective);
