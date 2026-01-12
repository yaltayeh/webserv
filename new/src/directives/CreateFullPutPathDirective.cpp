#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "configs/CommonConfig.hpp"

/**
 * @brief Parses the 'create_full_put_path' directive
 * Syntax: create_full_put_path on | off;
 * Example: create_full_put_path on;
 * Automatically creates directories for PUT requests
 */
struct Config *parseCreateFullPutPathDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("create_full_put_path", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("create_full_put_path", "requires exactly one argument (on or off)");

	if (args[1] == "on")
		config->create_full_put_path = true;
	else if (args[1] == "off")
		config->create_full_put_path = false;
	else
		throw Directive::ArgumentException("create_full_put_path", "invalid value '" + args[1] + "' (must be 'on' or 'off')");

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("create_full_put_path", parseCreateFullPutPathDirective);
