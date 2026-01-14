#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'cgi_extension' directive
 * Syntax: cgi_extension <ext1> <ext2> ...;
 * Example: cgi_extension .php .py;
 */
struct Config *parseCgiExtensionDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("cgi_extension", "can only be used in http, server or location context");

	// Parse extensions
	if (args.size() < 2)
		throw Directive::ArgumentException("cgi_extension", "requires at least one extension");

	// Clear existing extensions and add new ones
	config->cgi_extensions.clear();
	for (size_t i = 1; i < args.size(); i++)
	{
		config->cgi_extensions.push_back(args[i]);
	}

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("cgi_extension", parseCgiExtensionDirective);
