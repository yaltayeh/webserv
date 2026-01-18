#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Parser.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{

	Config *config_handler(Parser *parser, const std::vector<std::string> &args, Config *parent)
	{
		if (args.size() == 0)
			return NULL;
		DirectiveFactory::ParserFunc parserFunc = DirectiveFactory::getParser(args[0]);
		Config *config = parserFunc(parser, args, parent);
		return config;
	}

	/**
	 * @brief Constructs parser for entire string
	 */
	Parser::Parser(const std::string &configString, const Environ &environ)
		: _configString(configString), _environ(environ)
	{
		_start = _configString.c_str();
		_end = _start + _configString.length();
		_p = _start;
		_handler = config_handler;
	}

	/**
	 * @brief Copy constructor
	 * @param other Parser to copy from
	 * @note Recalculates pointers since _configString is copied
	 */
	Parser::Parser(const Parser &other)
		: _configString(other._configString), _environ(other._environ)
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

		_blockStart = other._blockStart;
		_blockEnd = other._blockEnd;
		_handler = other._handler;

		_filePath = other._filePath;
		_dirPath = other._dirPath;
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
		const_cast<Environ &>(_environ) = other._environ;

		_start = _configString.c_str();
		_end = _start + _configString.length();

		// Calculate offsets and apply to new string
		size_t pOffset = other._p - other._start;
		size_t directiveStartOffset = other._directiveStart - other._start;
		size_t directiveEndOffset = other._directiveEnd - other._start;

		_p = _start + pOffset;
		_directiveStart = _start + directiveStartOffset;
		_directiveEnd = _start + directiveEndOffset;

		_filePath = other._filePath;
		_dirPath = other._dirPath;

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
	 * @brief Opens configuration file and creates parser
	 * @throws FileException if file cannot be opened
	 */
	Parser Parser::openConfigFile(const std::string &fileName, const Environ &environ)
	{
		std::ifstream file(fileName.c_str());
		if (!file)
			throw FileException(fileName, "Cannot open configuration file");

		std::stringstream ss;
		ss << file.rdbuf();

		std::string str = ss.str();

		Parser parser = Parser(str, environ);
		parser._filePath = fileName;

		std::string dirpath;
		size_t last_slash_pos = fileName.find_last_of("/\\");
		if (last_slash_pos != std::string::npos)
			dirpath = fileName.substr(0, last_slash_pos);
		else
			dirpath = ".";
		parser._dirPath = dirpath;

		return (parser);
	}

	bool Parser::loadConfigFile(const std::string &fileName, Config &config, const Environ &environ)
	{
		try
		{

			Directive::Parser parser = Directive::Parser::openConfigFile(fileName, environ);
			while (parser.getNextDirectives())
			{
				parser.parseDirective(&config);
			}
			return true;
		}
		catch (const std::exception &e)
		{
			std::cerr << "\033[31mError: " << e.what() << "\033[0m" << std::endl;
			return false;
		}
	}

	void Parser::setHandler(ParserHandler handler)
	{
		_handler = handler;
	}

	Parser::ParserHandler Parser::getHandler() const
	{
		return _handler;
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

		_blockStart = NULL;
		_blockEnd = NULL;
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

		_directiveStart = p; // Mark start of directive

		// Find end of directive
		while (p < _end)
		{
			// skip qout and double qout
			if (*p == '\"' || *p == '\'')
				p = nextQout(p, _end, *p);
			else if (*p == '#')
				p = skipComment(p, _end);
			else if (*p == ';' && bracket_level == 0)
				break; // Simple directive end
			else if (*p == '$')
				p = skipVariableName(p, _end);
			else if (*p == '{')
			{
				if (bracket_level == 0)
					_blockStart = p;
				bracket_level++;
			}
			else if (*p == '}')
			{
				bracket_level--;
				if (bracket_level <= 0)
				{
					_blockEnd = p;
					break; // Block directive end
				}
			}
			p++;
		}
		if (bracket_level != 0)
			throw ParseException("Mismatched closing bracket != 0 '}'");
		if (p < _end && *p == '}')
			p++;
		_directiveEnd = p; // Mark end of directive
		if (p < _end && *p == ';')
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

	const std::string &Parser::getFilePath() const
	{
		return _filePath;
	}

	const std::string &Parser::getDirPath() const
	{
		return _dirPath;
	}

	const Environ &Parser::getEnviron() const
	{
		return _environ;
	}

	/**
	 * @brief Extracts directive name and arguments from text
	 * @return Vector with directive name as first element, followed by arguments
	 */
	std::vector<std::string> Parser::getArgs()
	{
		const char *p;
		const char *slice;
		const char *end;
		std::string name;
		std::vector<std::string> args;

		p = _directiveStart;
		end = _blockStart ? _blockStart - 1 : _directiveEnd;

		// split args
		while (p < end)
		{
			p = Directive::Parser::skipSpace(p, end);
			slice = p;
			while (p < end && !std::isspace(*p))
				p++;
			if (p > slice)
			{
				std::string arg = _configString.substr(slice - _configString.c_str(), p - slice);
				if (!args.empty())
					arg = ExpantionEnviron(arg);
				args.push_back(arg);
			}
			if (p >= end)
				break;
			p++;
		}
		return (args);
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

		Config *config = NULL;
		if (_handler)
			config = _handler(this, args, parent);
		if (config)
			config->parent = parent;
		if (_blockStart)
			parseBlockDirective(config);
		return config;
	}

	/**
	 * @brief Recursively parses block directive (directives inside { })
	 * @throws ParseException on syntax errors
	 */
	void Parser::parseBlockDirective(Config *parent)
	{
		const char *start;
		const char *end;

		start = _blockStart;
		end = _blockEnd;
		if (*start != '{' || *end != '}')
			throw ParseException("unexpected Block directive");
		start++;
		end--;
		if (end < start)
			return;
		std::string block = _configString.substr(start - _configString.c_str(), end - start);

		Directive::Parser parser(block, _environ);
		// std::cout << block << std::endl;
		parser.setHandler(_handler);
		parser._filePath = _filePath;
		parser._dirPath = _dirPath;
		while (parser.getNextDirectives())
			parser.parseDirective(parent);
		_blockStart = NULL;
		_blockEnd = NULL;
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

	const char *Parser::skipVariableName(const char *p, const char *end)
	{
		if (*p != '$')
			return (p);

		bool isEnv = false;
		p++; // skip $
		if (p >= end)
			throw SyntaxException("Unexpected end directive 1");
		if (*p == '{')
		{
			isEnv = true;
			p++;
			if (p >= end)
				throw SyntaxException("Unexpected end directive 1");
		}
		while (p < end && (std::isalnum(*p) || *p == '_')) // skip variable name
			p++;
		if (isEnv)
		{
			if (p >= end)
				throw SyntaxException("Unexpected end directive 2");
			if (*p != '}')
			{
				std::cout << "Unexpected char: " << *p << std::endl;
				throw SyntaxException("Unexpected char ");
			}
		}
		return (p);
	}

	std::string Parser::getEnvName(const std::string &str)
	{
		if (str.length() < 4)
			return ("");
		if (str[0] != '$' || str[1] != '{' || str[str.length() - 1] != '}')
			return ("");
		return str.substr(2, str.length() - 3);
	}

	const std::string &Parser::getVarValue(const std::string &str, size_t dollar_sign)
	{
		size_t i = dollar_sign + 1;
		bool isEnv = false;
		if (i < str.length() && str[i] == '{')
		{
			i++;
			isEnv = true;
		}
		size_t start = i;
		while (i < str.length())
		{
			if (!std::isalnum(str[i]) && str[i] != '_')
				break;
			i++;
		}
		if (i < str.length() && isEnv && str[i] != '}')
			throw SyntaxException("getVarVame: Unxpected char 2");
		if (i >= start)
			throw SyntaxException("getVarVame: Unxpected char 3");
		std::string name = str.substr(start, i);
		std::cout << "VarName: " << name << "      ";
		const std::string &value = _environ.getEnv(name);
		std::cout << "VarValue: " << value << std::endl;
		return (value);
	}

	std::string Parser::ExpantionEnviron(const std::string &raw)
	{
		std::stringstream ss;

		size_t i = 0;
		size_t dollar_sign;
		while (i < raw.length())
		{
			dollar_sign = raw.find('$', i);
			ss << raw.substr(i, dollar_sign - i);
			if (dollar_sign == std::string::npos)
				break;
			if (raw[dollar_sign + 1] == '{')
			{
				size_t startName = dollar_sign + 2;
				size_t endName = raw.find_first_of('}', startName);
				std::string name = raw.substr(startName, endName - startName);
				const std::string &value = _environ.getEnv(name);
				ss << value;
				i = endName + 1;
			}
			else
			{
				ss << raw[dollar_sign];
				i = dollar_sign + 1;
			}
		}
		std::string str;
		ss >> str;
		return (str);
	}

	std::string Parser::ExpantionVariable(const std::string &str)
	{
		(void)str;
		return std::string();
	}
}
