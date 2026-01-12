#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'upload_enable' directive
 * Syntax: upload_enable on | off;
 * Example: upload_enable on;
 */
struct Config *parseUploadEnableDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("upload_enable", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("upload_enable", "requires exactly one argument (on or off)");

	if (args[1] == "on")
		config->upload_enable = true;
	else if (args[1] == "off")
		config->upload_enable = false;
	else
		throw Directive::ArgumentException("upload_enable", "invalid value '" + args[1] + "' (must be 'on' or 'off')");

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("upload_enable", parseUploadEnableDirective);
