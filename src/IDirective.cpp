#include "IDirective.hpp"
#include <cstring>
#include "Parser.hpp"

std::unordered_map<std::string, IDirective::CreatorFunc> IDirective::registry;

IDirective::IDirective()
{
}

IDirective::IDirective(const std::vector<std::string> &args)
{
	(void)args;
}

IDirective::~IDirective()
{
}

std::vector<std::string> IDirective::getArgs(const std::string &str,
											 const char *start,
											 const char *end,
											 const char **block_start_ptr)
{

	const char *p;
	const char *slice;
	std::string name;
	std::vector<std::string> args;

	p = Config::Parser::getName(start, end, name);
	if (p == nullptr)
		return args;
	start = p;

	// std::cout << "name: " << name << std::endl;

	// Add name to args
	args.push_back(name);

	// parse args
	while (p < end)
	{
		p = Config::Parser::skipSpace(p, end);
		slice = p;
		while (p < end && !std::isspace(*p) && *p != '{' && *p != ';')
			p++;
		if (p > slice)
			args.push_back(str.substr(slice - str.c_str(), p - slice));
		if (p >= end || *p == '{' || *p == ';')
			break;
		p++;
	}
	if (*p == '{')
		*block_start_ptr = p;
	return (args);
}

const IDirective *IDirective::parseDirective(const IDirective *parent,
											 const std::string &str,
											 const char *start,
											 const char *end)
{
	std::vector<std::string> args;
	const char *block_start = NULL;

	(void)parent;
	args = IDirective::getArgs(str, start, end, &block_start);
	if (args.size() == 0)
	{
		// std::cerr << "Error Directive args.size() <= 0" << std::endl;
		// throw; // TODO
		return nullptr;
	}

	const std::string &name = args[0];
	try
	{
		IDirective::CreatorFunc &initFunction = IDirective::registry.at(name);
		IDirective *directive = initFunction(args);

		// change parent in directive
		directive->parent = parent;
		if (block_start)
			parseBlockDirective(directive, str, block_start, end);
		return directive;
	}
	catch (const std::out_of_range &e)
	{
		std::cerr << "\033[31mError Directive '" << name << "' Doesn't exist \033[0m" << std::endl;
		// throw; /* TODO: throw Directive not exist */
		return nullptr;
	}
}

void IDirective::parseBlockDirective(const IDirective *parent,
									 const std::string &str,
									 const char *start,
									 const char *end)
{
	const char *p = NULL;
	int bracket_level = 0;

	if (*start != '{')
		throw; // TODO parseBlockDirective sould start with {
	start++;   // skip first {
	bracket_level = 1;

	// skip spaces after {
	p = start;
	while (p < end && isspace(*p))
		p++;
	while (p < end)
	{
		// skip qout and double qout
		if (*p == '\"' || *p == '\'')
			p = Config::Parser::nextQout(p, end, *p);

		// skip comment
		if (*p == '#')
			p = Config::Parser::skipComment(p, end);

		if (*p == '{')
			bracket_level++;
		if (*p == '}')
			bracket_level--;
		if (*p == '}' && bracket_level == 0)
			break;
		if (bracket_level < 0)
			throw; // TODO: invalid close bracket } without open bracket {
		p++;
	}
	if (p >= end)
		throw; // if loop end without }
	end = p - 1;

	Config::Parser parser(str, start, end);

	while (parser.getNextDirectives())
	{
		std::string directive = parser.copyDirective();
		// std::cout << directive << std::endl;
		parseDirective(parent, directive);
		(void)parent;
	}
}

const IDirective *IDirective::parseDirective(const IDirective *parent, const std::string &str)
{
	return (parseDirective(parent, str, str.c_str(), str.c_str() + str.length()));
}

const std::type_info **IDirective::getAllowedInside()
{
	static const std::type_info *allowedInside[] = {
		NULL};
	return allowedInside;
}
