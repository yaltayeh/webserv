#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/HttpConfig.hpp"

/**
 * @brief Parses the 'server' directive (block directive)
 * Syntax: server { ... }
 * Example: server {
 *              listen 80;
 *              server_name example.com;
 *          }
 * Creates a virtual server configuration block
 */
struct Config *initServerConfig(const std::vector<std::string> &args, Config *parent)
{
	// Validate parent context - must be http
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("server", "can only be used in http context");

	// Server directive takes no arguments
	if (args.size() != 1)
		throw Directive::ArgumentException("server", "takes no arguments");

	// Create new server config block
	ServerConfig *server = new ServerConfig();
	http->servers.push_back(server);

	return server;
}

DEFINE_DIRECTIVE("server", initServerConfig);
