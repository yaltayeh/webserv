#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "configs/ServerConfig.hpp"

/**
 * @brief Parses the 'listen' directive
 * Syntax: listen address[:port] [default_server];
 * Example: listen 80;
 *          listen 127.0.0.1:8080;
 *          listen [::]:8080;
 */
struct Config *parseListenDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	ServerConfig *config = dynamic_cast<ServerConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("listen", "can only be used in server context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("listen", "requires only one argument");

	// Set value to config
	config->listen.push_back(args[1]);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("listen", parseListenDirective);
