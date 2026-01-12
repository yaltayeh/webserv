#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "configs/ServerConfig.hpp"

/**
 * @brief Parses the 'server_name' directive
 * Syntax: server_name name [...];
 * Example: server_name example.com www.example.com;
 */
struct Config *parseServerNameDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	ServerConfig *config = dynamic_cast<ServerConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("server_name", "can only be used in server context");

	// Parse value
	if (args.size() < 2)
		throw Directive::ArgumentException("server_name", "requires at least one argument");

	// Set value to config
	for (size_t i = 1; i < args.size(); i++)
		config->server_names.push_back(args[i]);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("server_name", parseServerNameDirective);
