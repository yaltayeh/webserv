#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'cgi_pass' directive
 * Syntax: cgi_pass <path_to_interpreter>;
 * Example: cgi_pass /usr/bin/python3;
 */
struct Config *parseCgiPassDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("cgi_pass", "can only be used in http, server or location context");

	// Parse interpreter path
	if (args.size() != 2)
		throw Directive::ArgumentException("cgi_pass", "requires exactly one argument (interpreter path)");

	config->cgi_pass = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("cgi_pass", parseCgiPassDirective);
