#include "RootDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: root allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(RootDirective, "root", HttpDirective, ServerDirective, LocationDirective)

	RootDirective* RootDirective::initRootDirective(const std::vector<std::string>& args)
	{
		// root requires exactly one argument (path)
		if (args.size() != 2)
		{
			throw ArgumentException("root", "takes exactly one argument (path)");
		}

		std::string path = args[1];
		
		// Validate path is not empty
		if (path.empty())
		{
			throw ArgumentException("root", "path cannot be empty");
		}

		return new RootDirective(path);
	}

	// Default constructor
	RootDirective::RootDirective()
		: IDirective(), path("html")
	{
	}

	// Parameterized constructor
	RootDirective::RootDirective(const std::string& path)
		: IDirective(), path(path)
	{
	}

	// Copy constructor
	RootDirective::RootDirective(const RootDirective& other)
		: IDirective(other), path(other.path)
	{
	}

	// Assignment operator
	RootDirective& RootDirective::operator=(const RootDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// path is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	RootDirective::~RootDirective()
	{
	}
}
