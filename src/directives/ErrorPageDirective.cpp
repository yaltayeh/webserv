#include "ErrorPageDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <cstdlib>

namespace Config
{
	// Register directive: error_page allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(ErrorPageDirective, "error_page", HttpDirective, ServerDirective, LocationDirective)

	ErrorPageDirective* ErrorPageDirective::initErrorPageDirective(const std::vector<std::string>& args)
	{
		// error_page requires at least: error_page code uri
		if (args.size() < 3)
		{
			throw ArgumentException("error_page", "requires at least one error code and a URI");
		}

		// Parse error codes (all arguments except the last one which is the URI)
		std::vector<int> codes;
		for (size_t i = 1; i < args.size() - 1; i++)
		{
			int code = std::atoi(args[i].c_str());
			
			// Validate HTTP status code range
			if (code < 300 || code > 599)
			{
				throw ArgumentException("error_page", "error code must be between 300-599");
			}
			
			codes.push_back(code);
		}

		// Last argument is the URI
		std::string uri = args[args.size() - 1];
		
		if (uri.empty())
		{
			throw ArgumentException("error_page", "URI cannot be empty");
		}

		return new ErrorPageDirective(codes, uri);
	}

	// Default constructor
	ErrorPageDirective::ErrorPageDirective()
		: IDirective(), codes(), uri("")
	{
	}

	// Parameterized constructor
	ErrorPageDirective::ErrorPageDirective(const std::vector<int>& codes, const std::string& uri)
		: IDirective(), codes(codes), uri(uri)
	{
	}

	// Copy constructor
	ErrorPageDirective::ErrorPageDirective(const ErrorPageDirective& other)
		: IDirective(other), codes(other.codes), uri(other.uri)
	{
	}

	// Assignment operator
	ErrorPageDirective& ErrorPageDirective::operator=(const ErrorPageDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// codes and uri are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	ErrorPageDirective::~ErrorPageDirective()
	{
	}
}
