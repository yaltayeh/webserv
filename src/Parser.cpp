#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Parser.hpp"

namespace Config
{

	Parser::Parser(const std::string &configString) : _configString(configString)
	{
		_start = _configString.c_str();
		_end = _start + _configString.length();
		_p = _start;
	}
	Parser::Parser(const std::string &configString,
				   const char *start,
				   const char *end) : _configString(configString)
	{
		_start = start;
		_end = end;
		_p = _start;
	}

	Parser Parser::openConfigFile(const char *fileName)
	{
		std::ifstream file(fileName);
		if (!file.is_open())
			throw std::runtime_error("Cannot open config file");

		std::stringstream ss;
		ss << file.rdbuf();

		return (Parser(ss.str()));
	}

	Parser::~Parser()
	{
	}

	bool Parser::getNextDirectives()
	{
		const char *p;
		int bracket_level = 0;
		const char *slice;

		if (_p >= _end)
			return false;
		p = _p;
		while (p < _end)
		{
			if (isspace(*p))
				p = skipSpace(p, _end);
			else if (*p == '#')
				p = skipComment(p, _end);
			else
				break;
		}
		slice = p;
		while (p < _end)
		{
			// skip qout and double qout
			if (*p == '\"' || *p == '\'')
				p = nextQout(p, _end, *p);
			if (*p == '#')
				p = skipComment(p, _end);
			if (*p == ';' && bracket_level == 0)
				break;
			if (*p == '{')
				bracket_level++;
			if (*p == '}')
				bracket_level--;
			if (*p == '}' && bracket_level == 0)
				break;
			if (bracket_level < 0)
				throw; // TODO: invalid close bracket }
			p++;
		}
		_directiveStart = slice;
		_directiveEnd = p;
		_p = (p < _end) ? p + 1 : p;
		return (true);
	}

	const char *Parser::getDirectiveStart() const
	{
		return _directiveStart;
	}

	const char *Parser::getDirectiveEnd() const
	{
		return _directiveEnd;
	}

	std::string Parser::copyDirective() const
	{
		return _configString.substr(_directiveStart - _configString.c_str(),
									_directiveEnd - _directiveStart + 1);
	}

	const char *Parser::skipSpace(const char *p, const char *end)
	{
		while (p < end && std::isspace(*p))
			p++;
		return p;
	}

	const char *Parser::skipComment(const char *p, const char *end)
	{
		p++;
		while (p < end && *p != '\n')
			p++;
		return p;
	}

	const char *Parser::nextQout(const char *p, const char *end, char qout)
	{
		p++; // skip first qout
		while (p < end && *p != qout)
			p++;
		if (*p != qout)
			return NULL;
		return p;
	}

	const char *Parser::getName(const char *p, const char *end, std::string &name)
	{
		const char *start;

		name = "";
		p = skipSpace(p, end);
		start = p;
		while (p < end)
		{
			if (std::islower(*p) || *p == '_')
				;
			else if (std::isdigit(*p))
				;
			else if (std::isspace(*p) || *p == ';' || *p == '{')
				break;
			else
				throw; /* TODO: throw unexpected char in directive name */
			name += *p++;
		}
		if (p <= start)
		{
			if (*p == '{')
				throw; // TODO: block directive without name
			return NULL;
		}
		return p;
	}
}
