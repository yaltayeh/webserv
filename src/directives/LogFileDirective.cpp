#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/HttpConfig.hpp"
#include "libutils/include/Logger.hpp"

/**
 * @brief Parses the 'log_file' directive
 * 
 * Syntax: log_file <filepath>;
 * 
 * Examples:
 *   log_file /var/log/webserv.log;
 *   log_file logs/access.log;
 *   log_file /dev/null;  # Disable logging
 * 
 * Notes:
 *   - File is opened in append mode
 *   - Creates parent directories if needed (not implemented yet)
 *   - If file cannot be opened, logs go to stdout/stderr
 */
struct Config *parseLogFileDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	
	// Can be used in http context
	HttpConfig *http = dynamic_cast<HttpConfig *>(parent);
	if (!http)
		throw Directive::InvalidNesting("log_file", "can only be used in http context");

	// Requires exactly one argument
	if (args.size() != 2)
		throw Directive::ArgumentException("log_file", "requires exactly one argument (file path)");

	std::string filepath = args[1];
	
	// Try to open the log file
	if (!Logger::openLogFile(filepath)) {
		throw Directive::ArgumentException("log_file", 
			"failed to open log file: '" + filepath + "'");
	}

	return parent;
}

DEFINE_DIRECTIVE("log_file", parseLogFileDirective);
