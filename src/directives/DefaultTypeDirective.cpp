#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <fstream>
#include <sstream>

struct Config *parseDefaultTypeDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("default_type", "can only be used in http, server or location context");
	if (args.size() != 2)
		throw Directive::ArgumentException("default_type", "requires exactly one argument (mime-type)");

	config->default_type = args[1];

	return NULL;
}

DEFINE_DIRECTIVE("default_type", parseDefaultTypeDirective);
