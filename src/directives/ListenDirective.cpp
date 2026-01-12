#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/ServerConfig.hpp"

#include <string>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>

ServerConfig::ListenAddr parse_listen(const std::string &input)
{
	ServerConfig::ListenAddr out;

	std::string host;
	std::string port_str;

	size_t colon = input.find(':');

	// Case: "80"
	if (colon == std::string::npos)
	{
		host = "*";
		port_str = input;
	}
	else
	{
		host = input.substr(0, colon);
		port_str = input.substr(colon + 1);
	}

	// ---- parse port ----
	char *end;
	long port = std::strtol(port_str.c_str(), &end, 10);
	if (*end != '\0' || port <= 0 || port > 65535)
		throw Directive::ArgumentException("listen", "Invalid port: " + port_str);

	out.port = static_cast<uint16_t>(port);

	// ---- parse address ----
	if (host.empty() || host == "*" || host == "0.0.0.0")
	{
		out.ip = htonl(INADDR_ANY);
		return out;
	}

	// IPv4 literal
	if (inet_pton(AF_INET, host.c_str(), &out.ip) == 1)
		return out;

	// Hostname
	struct hostent *he = gethostbyname(host.c_str());
	if (!he || he->h_addrtype != AF_INET)
		throw Directive::ArgumentException("listen", "Invalid address or hostname: " + host);

	std::memcpy(&out.ip, he->h_addr, sizeof(out.ip));
	return out;
}

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
	ServerConfig::ListenAddr addr = parse_listen(args[1]);
	if (config->is_new_listen == false)
	{
		config->listenAddresses.clear();
		config->is_new_listen = true;
	}
	config->listenAddresses.push_back(addr);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("listen", parseListenDirective);
