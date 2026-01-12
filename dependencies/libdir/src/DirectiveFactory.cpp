#include "DirectiveFactory.hpp"

std::map<std::string, DirectiveFactory::ParserFunc> &
DirectiveFactory::getRegistry()
{
	static std::map<std::string, ParserFunc> registry;
	return registry;
}
void DirectiveFactory::registerDirective(const std::string &name,
										 ParserFunc parser)
{
	getRegistry()[name] = parser;
}

DirectiveFactory::ParserFunc DirectiveFactory::getParser(const std::string &name)
{
	ParserFunc parser = getRegistry().at(name);
	return parser;
}