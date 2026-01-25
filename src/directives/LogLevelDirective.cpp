#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include "libutils/include/Logger.hpp"
#include <cctype>

/**
 * @brief Parses the 'log_level' directive
 * 
 * Syntax: log_level [level1] [level2] ... ;
 *         log_level [numeric_value];
 * 
 * String examples:
 *   log_level info;
 *   log_level debug info warning;
 *   log_level error critical;
 *   log_level all;
 *   log_level none;
 * 
 * Numeric examples:
 *   log_level 4;      # INFO only
 *   log_level 60;     # INFO | WARNING | ERROR | CRITICAL
 *   log_level 0x3C;   # INFO | WARNING | ERROR | CRITICAL (hex)
 *   log_level 63;     # ALL
 *   log_level 0;      # NONE
 * 
 * Bit values:
 *   trace    = 1   (0x01)
 *   debug    = 2   (0x02)
 *   info     = 4   (0x04)
 *   warning  = 8   (0x08)
 *   error    = 16  (0x10)
 *   critical = 32  (0x20)
 *   all      = 63  (0x3F)
 *   none     = 0
 */
struct Config *parseLogLevelDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	
	// Can be used in http context
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("log_level", "can only be used in http context");

	// Requires at least one argument
	if (args.size() < 2)
		throw Directive::ArgumentException("log_level", "requires at least one argument");

	int level_mask = 0;
	bool found_numeric = false;

	// Parse each argument
	for (size_t i = 1; i < args.size(); i++)
	{
		std::string arg = args[i];
		
		// Convert to lowercase for case-insensitive parsing
		std::string lower_arg;
		for (size_t j = 0; j < arg.length(); j++) {
			lower_arg += std::tolower(arg[j]);
		}

		// Try numeric parsing first
		int numeric = Logger::parseLevelNumber(lower_arg);
		if (numeric >= 0) {
			level_mask = numeric;  // Numeric value replaces everything
			found_numeric = true;
			break;  // Stop processing other arguments
		}

		// Try string parsing
		int string_level = Logger::parseLevelString(lower_arg);
		if (string_level >= 0) {
			level_mask |= string_level;  // OR together multiple levels
		} else {
			throw Directive::ArgumentException("log_level", 
				"invalid level: '" + arg + "' (valid: trace, debug, info, warning, error, critical, all, none, or numeric 0-63)");
		}
	}

	// If we found multiple string levels but then a numeric, warn about it
	if (found_numeric && args.size() > 2) {
		// Numeric overrides all string values - this is expected behavior
	}

	// Set the global log level
	Logger::g_logLevel = level_mask;

	return parent;
}

DEFINE_DIRECTIVE("log_level", parseLogLevelDirective);
