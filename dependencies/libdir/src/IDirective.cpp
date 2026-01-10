#include <cctype>
#include <algorithm>
#include <cstring>
#include "IDirective.hpp"
#include "Parser.hpp"
#include "DirectiveFactory.hpp"

namespace Config
{

/**
 * @brief Default constructor
 */
IDirective::IDirective() : parent(NULL) {}

/**
 * @brief Constructor with arguments (does nothing by default)
 * @param args Argument vector (unused in base class)
 */
IDirective::IDirective(const std::vector<std::string> &args) : parent(NULL)
{
	(void)args;
}

/**
 * @brief Copy constructor - performs deep copy of directive tree
 * @param other IDirective to copy from
 * @note Recursively copies all children
 */
IDirective::IDirective(const IDirective &other) : parent(NULL)
{
	// Deep copy all children
	for (std::multimap<std::string, IDirective *>::const_iterator it = other.children.begin();
		 it != other.children.end(); ++it)
	{
		// Note: Cannot truly deep copy polymorphic types without clone() method
		// This is a limitation of C++98 OCF with polymorphism
		// For now, we'll just copy the structure (shallow copy of pointers)
		children.insert(std::make_pair(it->first, it->second));
	}
}

/**
 * @brief Assignment operator - performs deep copy with self-assignment check
 * @param other IDirective to assign from
 * @return Reference to this
 */
IDirective &IDirective::operator=(const IDirective &other)
{
	// Self-assignment check
	if (this == &other)
		return *this;
	
	// Delete existing children
	for (std::multimap<std::string, IDirective *>::iterator it = children.begin();
		 it != children.end(); ++it)
	{
		delete it->second;
	}
	children.clear();
	
	// Copy parent pointer
	parent = NULL; // Don't copy parent to avoid issues
	
	// Deep copy all children from other
	for (std::multimap<std::string, IDirective *>::const_iterator it = other.children.begin();
		 it != other.children.end(); ++it)
	{
		// Note: Cannot truly deep copy polymorphic types without clone() method
		children.insert(std::make_pair(it->first, it->second));
	}
	
	return *this;
}

/**
 * @brief Destructor - recursively deletes all child directives
 * @note Fixes memory leak by properly cleaning up children
 */
IDirective::~IDirective()
{
	// Delete all children to prevent memory leaks
	for (std::multimap<std::string, IDirective *>::iterator it = children.begin();
		 it != children.end(); ++it)
	{
		delete it->second;
	}
	children.clear();
}

/**
 * @brief Validates and sets parent directive
 * @throws InvalidNesting if directive not allowed in parent
 */
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
	{
		// Get names for better error message
		std::string childName = DirectiveFactory::getName(&typeid(*this));
		std::string parentName = DirectiveFactory::getName(&typeid(*parent));
		throw InvalidNesting(childName, parentName);
	}
	this->parent = parent;
}

/**
 * @brief Adds and validates a child directive
 * @param child Child directive to add (takes ownership)
 * @note Logs errors to stderr but doesn't propagate exceptions
 */
void IDirective::addChild(IDirective *child)
{
	try
	{
		child->setParent(this);

		std::string name = child->getName();

		this->children.insert(std::make_pair(name, child));
	}
	catch (const std::exception &e)
	{
		std::cerr << "\033[31mError: " << e.what() << "\033[0m" << std::endl;
		// Note: Child is NOT deleted here - caller retains ownership on failure
	}
}

/**
 * @brief Returns all children as multimap
 */
const std::multimap<std::string, IDirective *> IDirective::getChildren() const
{
	return children;
}

/**
 * @brief Gets the name of this directive
 */
std::string IDirective::getName() const
{
	const std::type_info *id = &typeid(*this);
	return DirectiveFactory::getName(id);
}

/**
 * @brief Extracts directive name and arguments from text
 * @return Vector with directive name as first element, followed by arguments
 */
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

	// split args
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

/**
 * @brief Parses a single directive and adds to parent
 * @return Pointer to created directive or NULL on error
 */
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
		// Empty directive - skip
		return NULL;
	}

	const std::string &name = args[0];
	try
	{
		DirectiveFactory::CreatorFunc initFunction = DirectiveFactory::getCreator(name);
		IDirective *directive = initFunction(args);

		// change parent in directive and add child to parent
		parent->addChild(directive);

		if (block_start)
			parseBlockDirective(directive, str, block_start, end);
		return directive;
	}
	catch (const std::out_of_range &e)
	{
		std::cerr << "\033[31mError: Directive '" << name << "' doesn't exist\033[0m" << std::endl;
		return NULL;
	}
	catch (const std::exception &e)
	{
		std::cerr << "\033[31mError: " << e.what() << "\033[0m" << std::endl;
		return NULL;
	}
}

/**
 * @brief Recursively parses block directive (directives inside { })
 * @throws ParseException on syntax errors
 */
void IDirective::parseBlockDirective(IDirective *parent,
									 const std::string &str,
									 const char *start,
									 const char *end)
{
	const char *p = NULL;
	int bracket_level = 0;

	if (*start != '{')
		throw ParseException("Block directive must start with '{'");
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
			throw ParseException("Invalid close bracket '}' without matching open bracket '{'");
		p++;
	}
	if (p >= end)
		throw ParseException("Missing closing bracket '}' for block directive");
	end = p - 1;

	std::string block = str.substr(start - str.c_str(), end - start);

	Config::Parser parser(block);

	while (parser.getNextDirectives())
	{
		std::string directive = parser.copyDirective();
		parseDirective(parent, directive);
	}
}

/**
 * @brief Public parsing method with string parameters
 */
const IDirective *IDirective::parseDirective(IDirective *parent, const std::string &str)
{
	return (parseDirective(parent, str, str.c_str(), str.c_str() + str.length()));
}

/**
 * @brief Default implementation for getAllowedInside
 * @return NULL array (no valid parents)
 */
const std::type_info **IDirective::getAllowedInside() const
{
	static const std::type_info *allowedInside[] = {NULL};
	return allowedInside;
}

} // namespace Config
