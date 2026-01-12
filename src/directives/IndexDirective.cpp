#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'index' directive
 * Syntax: index file [...];
 * Example: index index.html index.htm;
 *          index index.php index.html;
 */
struct Config *parseIndexDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("index", "can only be used in http, server or location context");

	// Parse value
	if (args.size() < 2)
		throw Directive::ArgumentException("index", "requires at least one argument");

	// Set value to config
	config->index.clear();
	for (size_t i = 1; i < args.size(); i++)
		config->index.push_back(args[i]);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("index", parseIndexDirective);
