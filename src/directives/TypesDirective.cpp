#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <fstream>
#include <sstream>

Config *types_handler(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("types", "can only be used in http, server or location context");
	if (args.size() < 2)
		throw Directive::ArgumentException("types", "takes two MiME type and name extensions");

	const std::string &mimeType = args[0];
	for (std::vector<std::string>::const_iterator it = args.begin();
		 it != args.end();
		 it++)
	{
		config->mimeTypes[mimeType] = *it;
	}
	return NULL;
}

struct Config *parseTypesDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("types", "can only be used in http, server or location context");
	if (args.size() != 1)
		throw Directive::ArgumentException("types", "takes no arguments");
	config->mimeTypes.clear();

	Directive::Parser::ParserHandler oldHandler = parser->getHandler();
	parser->setHandler(types_handler);
	parser->parseBlockDirective(parent);
	parser->setHandler(oldHandler);
	return NULL;
}

DEFINE_DIRECTIVE("types", parseTypesDirective);
