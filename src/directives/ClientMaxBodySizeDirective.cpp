#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include "Config/HttpConfig.hpp"
#include "Config/ServerConfig.hpp"
#include <cstdlib>
#include <cctype>

/**
 * @brief Parses the 'client_max_body_size' directive
 * Syntax: client_max_body_size size;
 * Example: client_max_body_size 10m;
 *          client_max_body_size 1024;
 *          client_max_body_size 0; (unlimited)
 */
struct Config *parseClientMaxBodySizeDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config - only allow in http or server context (not location)
	CommonConfig *config = NULL;
	if (dynamic_cast<HttpConfig *>(parent) || dynamic_cast<ServerConfig *>(parent))
		config = dynamic_cast<CommonConfig *>(parent);
	
	if (!config)
		throw Directive::InvalidNesting("client_max_body_size", "can only be used in http or server context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("client_max_body_size", "requires exactly one argument");

	std::string size_str = args[1];
	size_t size = 0;
	size_t multiplier = 1;
	std::string numeric_part = size_str;

	// Check for size suffix (k, m, g)
	if (!size_str.empty())
	{
		char last = std::tolower(size_str[size_str.length() - 1]);
		switch (last)
		{
		case 'g':
			multiplier = 1024 * 1024 * 1024;
			numeric_part = size_str.substr(0, size_str.length() - 1);
			break;
		case 'm':
			multiplier = 1024 * 1024;
			numeric_part = size_str.substr(0, size_str.length() - 1);
			break;
		case 'k':
			multiplier = 1024;
			numeric_part = size_str.substr(0, size_str.length() - 1);
			break;
		default:
			// No suffix, all characters should be digits
			break;
		}
	}

	// Validate that numeric_part contains only digits
	if (numeric_part.empty())
		throw Directive::ArgumentException("client_max_body_size", "invalid size format: empty value");
	
	for (size_t i = 0; i < numeric_part.length(); i++)
	{
		if (!std::isdigit(numeric_part[i]))
			throw Directive::ArgumentException("client_max_body_size", "invalid size format: '" + size_str + "' (must be a number optionally followed by k, m, or g)");
	}

	size = std::atoll(numeric_part.c_str()) * multiplier;

	// Set value to config
	config->client_max_body_size = size;

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("client_max_body_size", parseClientMaxBodySizeDirective);
