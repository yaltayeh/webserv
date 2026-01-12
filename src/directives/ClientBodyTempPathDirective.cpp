#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'client_body_temp_path' directive
 * Syntax: client_body_temp_path path;
 * Example: client_body_temp_path /var/tmp/nginx/client_body;
 * Sets the temporary directory for storing client request bodies
 */
struct Config *parseClientBodyTempPathDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("client_body_temp_path", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("client_body_temp_path", "requires exactly one argument");

	// Set value to config
	config->client_body_temp_path = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("client_body_temp_path", parseClientBodyTempPathDirective);
