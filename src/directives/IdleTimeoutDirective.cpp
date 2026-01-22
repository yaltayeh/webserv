#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include <cstdlib>
#include <cctype>

/**
 * @brief Parses the 'idle_timeout' directive
 * Syntax: idle_timeout time;
 * Example: idle_timeout 75;
 *          idle_timeout 75s; (seconds)
 *          idle_timeout 5m;  (minutes)
 *          idle_timeout 2h;  (hours)
 *          idle_timeout 1d;  (days)
 *          idle_timeout 0;   (disable)
 */
struct Config *parseIdleTimeoutDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("idle_timeout", "can only be used in http context");

	// Parse value
	if (args.size() != 2)
		throw Directive::ArgumentException("idle_timeout", "requires exactly one argument");

	std::string time_str = args[1];
	size_t timeout = 0;
	size_t multiplier = 1;

	// Check for time suffix and get multiplier
	if (!time_str.empty())
	{
		char suffix = std::tolower(time_str[time_str.length() - 1]);
		if (!std::isdigit(suffix))
		{
			switch (suffix)
			{
				case 's':
					multiplier = 1;
					break;
				case 'm':
					multiplier = 60;
					break;
				case 'h':
					multiplier = 3600;
					break;
				case 'd':
					multiplier = 86400;
					break;
				default:
					throw Directive::ArgumentException("idle_timeout", "invalid time suffix (use s, m, h, or d)");
			}
			time_str = time_str.substr(0, time_str.length() - 1);
		}
	}

	// Convert to number
	timeout = std::atoll(time_str.c_str()) * multiplier;

	// Set value to http
	http->idle_timeout = timeout;

	// Return no new http
	return NULL;
}

DEFINE_DIRECTIVE("idle_timeout", parseIdleTimeoutDirective);
