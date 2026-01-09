#include "ReturnDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <cstdlib>

namespace Config
{
	// Register directive: return allowed in ServerDirective and LocationDirective
	DEFINE_DIRECTIVE(ReturnDirective, "return", ServerDirective, LocationDirective)

	ReturnDirective* ReturnDirective::initReturnDirective(const std::vector<std::string>& args)
	{
		// return requires: return code [text|URL]
		if (args.size() < 2)
		{
			throw ArgumentException("return", "requires at least status code");
		}

		int code = std::atoi(args[1].c_str());
		
		// Validate HTTP status code range
		if (code < 100 || code > 599)
		{
			throw ArgumentException("return", "invalid HTTP status code");
		}

		// Optional text/URL
		std::string text = "";
		if (args.size() > 2)
		{
			text = args[2];
		}

		return new ReturnDirective(code, text);
	}

	// Default constructor
	ReturnDirective::ReturnDirective()
		: IDirective(), code(200), text("")
	{
	}

	// Parameterized constructor
	ReturnDirective::ReturnDirective(int code, const std::string& text)
		: IDirective(), code(code), text(text)
	{
	}

	// Copy constructor
	ReturnDirective::ReturnDirective(const ReturnDirective& other)
		: IDirective(other), code(other.code), text(other.text)
	{
	}

	// Assignment operator
	ReturnDirective& ReturnDirective::operator=(const ReturnDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// code and text are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	ReturnDirective::~ReturnDirective()
	{
	}
}
