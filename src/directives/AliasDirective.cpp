#include "AliasDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: alias allowed only in LocationDirective
	DEFINE_DIRECTIVE(AliasDirective, "alias", LocationDirective)

	AliasDirective* AliasDirective::initAliasDirective(const std::vector<std::string>& args)
	{
		// alias requires exactly one argument (path)
		if (args.size() != 2)
		{
			throw ArgumentException("alias", "takes exactly one argument (path)");
		}

		std::string path = args[1];
		
		// Validate path is not empty
		if (path.empty())
		{
			throw ArgumentException("alias", "path cannot be empty");
		}

		return new AliasDirective(path);
	}

	// Default constructor
	AliasDirective::AliasDirective()
		: IDirective(), path("")
	{
	}

	// Parameterized constructor
	AliasDirective::AliasDirective(const std::string& path)
		: IDirective(), path(path)
	{
	}

	// Copy constructor
	AliasDirective::AliasDirective(const AliasDirective& other)
		: IDirective(other), path(other.path)
	{
	}

	// Assignment operator
	AliasDirective& AliasDirective::operator=(const AliasDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// path is const, cannot be reassigned
		}
		return *this;
	}

	// Destructor
	AliasDirective::~AliasDirective()
	{
	}
}
