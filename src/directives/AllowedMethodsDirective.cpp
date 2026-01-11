#include "AllowedMethodsDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: allowed_methods allowed in LocationDirective
	DEFINE_DIRECTIVE(AllowedMethodsDirective, "allowed_methods", LocationDirective)

	AllowedMethodsDirective* AllowedMethodsDirective::initAllowedMethodsDirective(const std::vector<std::string>& args)
	{
		// allowed_methods requires at least one HTTP method
		if (args.size() < 2)
		{
			throw ArgumentException("allowed_methods", "requires at least one HTTP method");
		}

		// Collect all methods
		std::vector<std::string> methods;
		for (size_t i = 1; i < args.size(); i++)
		{
			std::string method = args[i];
			
			// Validate common HTTP methods (can be extended)
			if (method != "GET" && method != "POST" && method != "PUT" && 
			    method != "DELETE" && method != "HEAD" && method != "OPTIONS" &&
			    method != "PATCH" && method != "CONNECT" && method != "TRACE")
			{
				throw ArgumentException("allowed_methods", "invalid HTTP method: " + method);
			}
			
			methods.push_back(method);
		}

		return new AllowedMethodsDirective(methods);
	}

	// Default constructor
	AllowedMethodsDirective::AllowedMethodsDirective()
		: IDirective(), methods()
	{
	}

	// Parameterized constructor
	AllowedMethodsDirective::AllowedMethodsDirective(const std::vector<std::string>& methods)
		: IDirective(), methods(methods)
	{
	}

	// Copy constructor
	AllowedMethodsDirective::AllowedMethodsDirective(const AllowedMethodsDirective& other)
		: IDirective(other), methods(other.methods)
	{
	}

	// Assignment operator
	AllowedMethodsDirective& AllowedMethodsDirective::operator=(const AllowedMethodsDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// methods is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	AllowedMethodsDirective::~AllowedMethodsDirective()
	{
	}
}
