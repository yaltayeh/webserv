#pragma once

#include <vector>
#include <map>
#include <string>
#include "Config.hpp"
#include "Parser.hpp"

class DirectiveFactory
{
public:
	typedef Config *(*ParserFunc)(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent);
	static void registerDirective(const std::string &name,
								  ParserFunc parser);
	static ParserFunc getParser(const std::string &name);

private:
	static std::map<std::string, ParserFunc> &getRegistry();
};

#define DEFINE_DIRECTIVE(_name, parser)                         \
	struct _##parser##Registrar                                 \
	{                                                            \
		_##parser##Registrar()                                  \
		{                                                        \
			DirectiveFactory::registerDirective(_name, parser); \
		}                                                        \
	} _##parser##Registrar;
