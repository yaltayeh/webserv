#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
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
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("client_max_body_size", "can only be used in http, server or location context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("client_max_body_size", "requires exactly one argument");

	std::string size_str = args[1];
	size_t size = 0;
	size_t multiplier = 1;

	// Check for size suffix (k, m, g)
	if (!size_str.empty())
	{
		char last = std::tolower(size_str[size_str.length() - 1]);
		switch (last)
		{
		case 'g':
			multiplier = 1024 * 1024 * 1024;
			break;
		case 'm':
			multiplier = 1024 * 1024;
			break;
		case 'k':
			multiplier = 1024;
			break;
		default:
			break;
		}
	}

	size = std::atoll(size_str.c_str()) * multiplier;

	// Set value to config
	config->client_max_body_size = size;

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("client_max_body_size", parseClientMaxBodySizeDirective);
