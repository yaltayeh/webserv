#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Parser.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{

	/**
	 * @brief Constructs parser for entire string
	 */
	Parser::Parser(std::string configString) : _configString(configString)
	{
		_start = _configString.c_str();
		_end = _start + _configString.length();
		_p = _start;
		config = NULL;
	}

	/**
	 * @brief Copy constructor
	 * @param other Parser to copy from
	 * @note Recalculates pointers since _configString is copied
	 */
	Parser::Parser(const Parser &other) : _configString(other._configString)
	{
		_start = _configString.c_str();
		_end = _start + _configString.length();

		// Calculate offsets and apply to new string
		size_t pOffset = other._p - other._start;
		size_t directiveStartOffset = other._directiveStart - other._start;
		size_t directiveEndOffset = other._directiveEnd - other._start;

		_p = _start + pOffset;
		_directiveStart = _start + directiveStartOffset;
		_directiveEnd = _start + directiveEndOffset;
	}

	/**
	 * @brief Assignment operator
	 * @param other Parser to assign from
	 * @return Reference to this
	 */
	Parser &Parser::operator=(const Parser &other)
	{
		// Self-assignment check
		if (this == &other)
			return *this;

		// Note: _configString is const, so we can't reassign it
		// This is a design limitation - Parser should be non-copyable
		// or _configString should not be const
		// For now, we'll use const_cast (not ideal but necessary for OCF)
		const_cast<std::string &>(_configString) = other._configString;

		_start = _configString.c_str();
		_end = _start + _configString.length();

		// Calculate offsets and apply to new string
		size_t pOffset = other._p - other._start;
		size_t directiveStartOffset = other._directiveStart - other._start;
		size_t directiveEndOffset = other._directiveEnd - other._start;

		_p = _start + pOffset;
		_directiveStart = _start + directiveStartOffset;
		_directiveEnd = _start + directiveEndOffset;

		return *this;
	}

	/**
	 * @brief Destructor
	 * @note Nothing to clean up - string is managed by std::string
	 */
	Parser::~Parser()
	{
		// Nothing to clean up
	}

	/**
	 * @brief Constructs parser for substring
	 */
	Parser::Parser(std::string configString,
				   size_t start,
				   size_t len) : _configString(configString)
	{
		_start = _configString.c_str() + start;
		_end = _start + len;
		_p = _start;
	}

	/**
	 * @brief Opens configuration file and creates parser
	 * @throws FileException if file cannot be opened
	 */
	Parser Parser::openConfigFile(const std::string &fileName)
	{
		std::ifstream file(fileName);
		if (!file)
			throw FileException(fileName, "Cannot open configuration file");

		std::stringstream ss;
		ss << file.rdbuf();

		std::string str = ss.str();

		return (Parser(str));
	}

	bool Parser::loadConfigFile(const std::string &fileName, Config &config)
	{
		try
		{

			Directive::Parser parser = Directive::Parser::openConfigFile(fileName);
			while (parser.getNextDirectives())
			{
				parser.parseDirective(&config);
			}
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	/**
	 * @brief Extracts next directive from configuration
	 * @return true if directive found, false if at end
	 * @throws ParseException on mismatched brackets
	 */
	bool Parser::getNextDirectives()
	{
		const char *p;
		int bracket_level = 0;
		const char *slice;

		_block_start = NULL;
		if (_p >= _end)
			return false;
		p = _p;

		// Skip leading whitespace and comments
		while (p < _end)
		{
			if (isspace(*p))
				p = skipSpace(p, _end);
			else if (*p == '#')
				p = skipComment(p, _end);
			else
				break;
		}

		slice = p; // Mark start of directive

		// Find end of directive
		while (p < _end)
		{
			// skip qout and double qout
			if (*p == '\"' || *p == '\'')
				p = nextQout(p, _end, *p);
			if (*p == '#')
				p = skipComment(p, _end);
			if (*p == ';' && bracket_level == 0)
				break; // Simple directive end
			if (*p == '{')
			{
				if (bracket_level == 0)
					_block_start = p;
				bracket_level++;
			}
			if (*p == '}')
			{
				bracket_level--;
				if (bracket_level == 0)
					break; // Block directive end
			}
			if (bracket_level < 0)
				throw ParseException("Mismatched closing bracket '}'");
			p++;
		}
		if (bracket_level != 0)
			throw ParseException("Mismatched closing bracket '}'");
		_directiveStart = slice;
		if (*p == '}')
			p++;
		_directiveEnd = p;
		if (*p == ';')
			p++;
		_p = p;
		return (true);
	}

	/**
	 * @brief Returns current directive text
	 */
	const char *Parser::getDirectiveStart() const
	{
		return _directiveStart;
	}

	/**
	 * @brief Returns pointer to end of current directive
	 */
	const char *Parser::getDirectiveEnd() const
	{
		return _directiveEnd;
	}

	/**
	 * @brief Extracts directive name and arguments from text
	 * @return Vector with directive name as first element, followed by arguments
	 */
	std::vector<std::string> Parser::getArgs()
	{
		const char *p;
		const char *slice;
		std::string name;
		std::vector<std::string> args;

		p = Directive::Parser::getName(_directiveStart, _directiveEnd, name);
		if (p == NULL)
			return args;

		// Add name to args
		args.push_back(name);

		// split args
		while (p < _directiveEnd)
		{
			p = Directive::Parser::skipSpace(p, _directiveEnd);
			slice = p;
			while (p < _directiveEnd && !std::isspace(*p) && *p != '{' && *p != ';')
				p++;
			if (p > slice)
				args.push_back(_configString.substr(slice - _configString.c_str(), p - slice));
			if (p >= _directiveEnd || *p == '{' || *p == ';')
				break;
			p++;
		}
		return (args);
	}

	const Config *Parser::parseConfig(Parser *parser) {
		(void)parser;
		return (NULL);
	}

	/**
	 * @brief Parses a single directive and adds to parent
	 * @return Pointer to created directive or NULL on error
	 */
	const Config *Parser::parseDirective(Config *parent)
	{
		std::vector<std::string> args;

		args = Parser::getArgs();
		if (args.size() == 0)
		{
			// Empty directive - skip
			return NULL;
		}

		const std::string &name = args[0];
		try
		{
			// parseConfig(this);
			DirectiveFactory::ParserFunc parserFunc = DirectiveFactory::getParser(name);
			Config *config = parserFunc(NULL, args, parent);
			if (config)
				config->parent = parent;
			if (_block_start)
				parseBlockDirective(config);
			return config;
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
	void Parser::parseBlockDirective(Config *parent)
	{
		const char *p = NULL;
		const char *start;
		const char *end;
		int bracket_level = 0;

		start = _block_start;
		end = _directiveEnd;
		if (*start != '{')
			throw ParseException("Block directive must start with '{'");
		start++; // skip first {
		bracket_level = 1;

		// skip spaces after {
		p = start;
		while (p < end && isspace(*p))
			p++;
		while (p < end)
		{
			// skip qout and double qout
			if (*p == '\"' || *p == '\'')
				p = Directive::Parser::nextQout(p, end, *p);

			// skip comment
			if (*p == '#')
				p = Directive::Parser::skipComment(p, end);

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

		std::string block = _configString.substr(start - _configString.c_str(), end - start);

		Directive::Parser parser(block);
		std::cout << "block: " << block << std::endl;

		while (parser.getNextDirectives())
			parser.parseDirective(parent);
	}

	/**
	 * @brief Skips whitespace characters
	 */
	const char *Parser::skipSpace(const char *p, const char *end)
	{
		while (p < end && std::isspace(*p))
			p++;
		return p;
	}

	/**
	 * @brief Skips from '#' to end of line
	 */
	const char *Parser::skipComment(const char *p, const char *end)
	{
		p++; // Skip '#'
		while (p < end && *p != '\n')
			p++;
		return p;
	}

	/**
	 * @brief Advances past closing quote, handling escape sequences
	 * @note NOW FIXED: Properly handles \\", \\', \\\\, and other escapes
	 * @return Pointer to closing quote, or NULL if unclosed
	 */
	const char *Parser::nextQout(const char *p, const char *end, char qout)
	{
		p++; // skip opening quote
		while (p < end && *p != qout)
		{
			// Handle escape sequences
			if (*p == '\\' && (p + 1) < end)
			{
				// Skip escaped character (including escaped quotes and backslashes)
				p += 2;
			}
			else
			{
				p++;
			}
		}
		if (p >= end || *p != qout)
			throw SyntaxException("Unclosed quote");
		return p;
	}

	/**
	 * @brief Extracts and validates directive name
	 * @throws SyntaxException on invalid characters or malformed name
	 */
	const char *Parser::getName(const char *p, const char *end, std::string &name)
	{
		const char *start;

		name = "";
		p = skipSpace(p, end);
		start = p;

		// Parse directive name (lowercase, underscore, digits after first char)
		while (p < end)
		{
			if (std::islower(*p) || *p == '_')
				; // Valid character
			else if (std::isdigit(*p))
				; // Valid after first character
			else if (std::isspace(*p) || *p == ';' || *p == '{')
				break; // End of name
			else
				throw SyntaxException(std::string("Invalid character '") + *p + "' in directive name");
			name += *p++;
		}

		// Check for errors
		if (p <= start)
		{
			if (p < end && *p == '{')
				throw SyntaxException("Block directive without name");
			return NULL; // No name found
		}

		return p;
	}
}
