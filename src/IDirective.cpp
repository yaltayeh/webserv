#include "IDirective.hpp"
#include <cstring>
#include "Parser.hpp"

std::map<std::string, IDirective::CreatorFunc> IDirective::registry_name;
std::map<const std::type_info *, IDirective::CreatorFunc> IDirective::registry_id;

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

void IDirective::setParent(IDirective *parent)
{
	const std::type_info **allowedInside;
	size_t i;

	allowedInside = getAllowedInside();
	i = 0;
	for (i = 0; allowedInside[i]; i++)
	{
		if (*allowedInside[i] == typeid(*parent))
			break;
	}
	if (allowedInside[i] == NULL)
		throw std::runtime_error("you can't typeid(*this) inside (typeid(*parent).name()) block"); // you can't typeid(*this) inside (typeid(*parent).name()) block
	this->parent = parent;
}

void IDirective::addChild(IDirective *child)
{
	try
	{
		child->setParent(this);

		this->children.insert(std::make_pair(child->getName(), child));
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

const std::string IDirective::getName() const
{
	return ("");
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
	if (p == NULL)
		return args;
	start = p;

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

const IDirective *IDirective::parseDirective(IDirective *parent,
											 const std::string &str,
											 const char *start,
											 const char *end)
{
	std::vector<std::string> args;
	const char *block_start = NULL;

	args = IDirective::getArgs(str, start, end, &block_start);
	if (args.size() == 0)
	{
		// std::cerr << "Error Directive args.size() <= 0" << std::endl;
		// throw; // TODO
		return NULL;
	}

	const std::string &name = args[0];
	try
	{
		const IDirective::CreatorFunc &initFunction = IDirective::getCreator(name);
		IDirective *directive = initFunction(args);
		
		// change parent in directive and add child to parent
		parent->addChild(directive);

		if (block_start)
			parseBlockDirective(directive, str, block_start, end);
		return directive;
	}
	catch (const std::out_of_range &e)
	{
		std::cerr << "\033[31mError Directive '" << name << "' Doesn't exist \033[0m" << std::endl;
		// throw; /* TODO: throw Directive not exist */
		return NULL;
	}
}

void IDirective::parseBlockDirective(IDirective *parent,
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

	std::string block = str.substr(start - str.c_str(), end - start);

	Config::Parser parser(block);

	while (parser.getNextDirectives())
	{
		std::string directive = parser.copyDirective();
		parseDirective(parent, directive);
	}
}

const IDirective *IDirective::parseDirective(IDirective *parent, const std::string &str)
{
	return (parseDirective(parent, str, str.c_str(), str.c_str() + str.length()));
}

void IDirective::addRegistry(std::string name, const std::type_info *id, const IDirective::CreatorFunc &creator)
{
	registry_name[name] = creator;
	registry_id[id] = creator;
}

IDirective::CreatorFunc IDirective::getCreator(std::string name)
{
	return registry_name.at(name);
}

IDirective::CreatorFunc IDirective::getCreator(const std::type_info *id)
{
	return registry_id.at(id);
}

const std::type_info **IDirective::getAllowedInside() const
{
	static const std::type_info *allowedInside[] = {NULL};
	return allowedInside;
}
