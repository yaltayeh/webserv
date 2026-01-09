#include "LocationDirective.hpp"
#include "ServerDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: location allowed in ServerDirective and LocationDirective (can be nested)
	DEFINE_DIRECTIVE(LocationDirective, "location", ServerDirective, LocationDirective)

	LocationDirective* LocationDirective::initLocationDirective(const std::vector<std::string>& args)
	{
		// location requires at least uri argument
		// Syntax: location [modifier] uri
		if (args.size() < 2)
		{
			throw ArgumentException("location", "requires at least URI argument");
		}

		std::string modifier = "";
		std::string uri = "";

		if (args.size() == 2)
		{
			// location uri
			uri = args[1];
		}
		else if (args.size() == 3)
		{
			// location modifier uri
			std::string firstArg = args[1];
			
			// Check if first arg is a valid modifier
			if (firstArg == "=" || firstArg == "~" || firstArg == "~*" || 
			    firstArg == "^~" || firstArg[0] == '@')
			{
				modifier = firstArg;
				uri = args[2];
			}
			else
			{
				// Not a modifier, treat as error (too many args)
				throw ArgumentException("location", "takes one or two arguments (optional modifier and URI)");
			}
		}
		else
		{
			throw ArgumentException("location", "takes one or two arguments (optional modifier and URI)");
		}

		// Validate URI is not empty
		if (uri.empty())
		{
			throw ArgumentException("location", "URI cannot be empty");
		}

		return new LocationDirective(modifier, uri);
	}

	// Default constructor
	LocationDirective::LocationDirective()
		: IDirective(), modifier(""), uri("/")
	{
	}

	// Parameterized constructor
	LocationDirective::LocationDirective(const std::string& modifier, const std::string& uri)
		: IDirective(), modifier(modifier), uri(uri)
	{
	}

	// Copy constructor
	LocationDirective::LocationDirective(const LocationDirective& other)
		: IDirective(other), modifier(other.modifier), uri(other.uri)
	{
	}

	// Assignment operator
	LocationDirective& LocationDirective::operator=(const LocationDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// modifier and uri are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	LocationDirective::~LocationDirective()
	{
	}
}
