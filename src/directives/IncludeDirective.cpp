#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Parser.hpp"
#include <fstream>
#include <sstream>

/**
 * @brief Parses the 'include' directive
 * Syntax: include file | mask;
 * Example: include mime.types;
 *          include fastcgi_params;
 *          include sites-enabled/ followed by *.conf;
 * 
 * Note: This is a simplified implementation that doesn't actually parse
 * the included file. In a full implementation, you would:
 * 1. Read the included file
 * 2. Parse its contents
 * 3. Insert directives into current context
 * 
 * For now, this just validates the syntax.
 */
struct Config *parseIncludeDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	if (args.size() != 2)
		throw Directive::ArgumentException("include", "requires exactly one argument (file path)");
	
	Directive::Parser::loadConfigFile(args[1], *parent);

	return NULL;
}

DEFINE_DIRECTIVE("include", parseIncludeDirective);
