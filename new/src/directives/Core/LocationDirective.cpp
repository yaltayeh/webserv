#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "configs/CommonConfig.hpp"
#include "configs/LocationConfig.hpp"

/**
 * @brief Parses the 'location' directive (block directive)
 * Syntax: location path { ... }
 * Example: location / { }
 *          location /api { }
 * Creates a new location context for URL path matching
 */
struct Config *parseLocationDirective(const std::vector<std::string> &args, Config *parent)
{
	CommonConfigServerLocation *config = dynamic_cast<CommonConfigServerLocation *>(parent);
	if (!config)
		throw Directive::InvalidNesting("location", "can only be used in server or location context");

	if (args.size() != 2)
		throw Directive::ArgumentException("location", "requires exactly one argument (path)");
	
	// Create new location config
	LocationConfig *loc = new LocationConfig(parent, args[1]);
	config->locations.push_back(loc);
	return loc;
}

DEFINE_DIRECTIVE("location", parseLocationDirective);
