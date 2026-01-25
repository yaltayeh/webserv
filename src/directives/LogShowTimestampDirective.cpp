#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include "libutils/include/Logger.hpp"
#include <cctype>

/**
 * @brief Parses the 'log_show_timestamp' directive
 * 
 * Syntax: log_show_timestamp on|off;
 * 
 * Examples:
 *   log_show_timestamp on;   # Show [YYYY-MM-DD HH:MM:SS] in logs (default)
 *   log_show_timestamp off;  # Hide timestamp for minimal logs
 */
struct Config *parseLogShowTimestampDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	
	// Can be used in http context
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("log_show_timestamp", "can only be used in http context");

	// Requires exactly one argument
	if (args.size() != 2)
		throw Directive::ArgumentException("log_show_timestamp", "requires exactly one argument (on or off)");

	std::string value = args[1];
	
	// Convert to lowercase
	std::string lower_value;
	for (size_t i = 0; i < value.length(); i++) {
		lower_value += std::tolower(value[i]);
	}

	// Parse on/off/true/false/1/0
	if (lower_value == "on" || lower_value == "true" || lower_value == "1") {
		Logger::g_logShowTimestamp = true;
	} else if (lower_value == "off" || lower_value == "false" || lower_value == "0") {
		Logger::g_logShowTimestamp = false;
	} else {
		throw Directive::ArgumentException("log_show_timestamp", 
			"invalid value: '" + value + "' (expected: on, off, true, false, 1, or 0)");
	}

	return parent;
}

DEFINE_DIRECTIVE("log_show_timestamp", parseLogShowTimestampDirective);
