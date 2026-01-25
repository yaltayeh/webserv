#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include <cstdlib>
#include <cerrno>
#include <climits>

/**
 * @brief Parses the 'worker_connections' directive
 * 
 * Syntax: worker_connections [number];
 * 
 * Examples:
 *   worker_connections 1024;
 *   worker_connections 512;
 *   worker_connections 4096;
 * 
 * Default: 1024
 * Minimum: 1
 * Maximum: INT_MAX
 */
struct Config *parseWorkerConnectionsDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	
	// Can only be used in http context
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("worker_connections", "can only be used in http context");

	// Requires exactly one argument
	if (args.size() != 2)
		throw Directive::ArgumentException("worker_connections", "requires exactly one argument");

	const std::string &value = args[1];

	// Validate numeric input
	for (size_t i = 0; i < value.length(); i++)
	{
		if (!isdigit(value[i]))
			throw Directive::ArgumentException("worker_connections", "argument must be a positive integer");
	}

	// Convert to integer with overflow protection
	errno = 0;
	char *endptr;
	long num = std::strtol(value.c_str(), &endptr, 10);

	if (errno == ERANGE || num > INT_MAX || num < 1)
		throw Directive::ArgumentException("worker_connections", "value out of range (must be between 1 and 2147483647)");

	if (*endptr != '\0')
		throw Directive::ArgumentException("worker_connections", "invalid numeric value");

	http->worker_connections = static_cast<int>(num);

	return parent;
}

DEFINE_DIRECTIVE("worker_connections", parseWorkerConnectionsDirective);
