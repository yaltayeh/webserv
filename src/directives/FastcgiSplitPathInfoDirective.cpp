#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/LocationConfig.hpp"

/**
 * @brief Parses the 'fastcgi_split_path_info' directive
 * Syntax: fastcgi_split_path_info regex;
 * Example: fastcgi_split_path_info ^(.+\.php)(/.+)$;
 * Defines a regular expression that captures SCRIPT_NAME and PATH_INFO
 */
struct Config *parseFastcgiSplitPathInfoDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	LocationConfig *config = dynamic_cast<LocationConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("fastcgi_split_path_info", "can only be used in location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("fastcgi_split_path_info", "requires exactly one argument (regex pattern)");

	// Set value to config
	config->fastcgi_split_path_info = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("fastcgi_split_path_info", parseFastcgiSplitPathInfoDirective);
