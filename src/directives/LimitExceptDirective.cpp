#include "LimitExceptDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: limit_except allowed only in LocationDirective (it's a block directive)
	DEFINE_DIRECTIVE(LimitExceptDirective, "limit_except", LocationDirective)

	LimitExceptDirective* LimitExceptDirective::initLimitExceptDirective(const std::vector<std::string>& args)
	{
		// limit_except requires at least one method
		// Syntax: limit_except METHOD1 [METHOD2 ...] { ... }
		if (args.size() < 2)
		{
			throw ArgumentException("limit_except", "requires at least one HTTP method");
		}

		std::vector<std::string> methods;
		
		// Valid HTTP methods
		const char* validMethods[] = {
			"GET", "POST", "PUT", "DELETE", "HEAD", 
			"OPTIONS", "PATCH", "CONNECT", "TRACE", NULL
		};

		// Collect and validate methods (args[1] onwards)
		for (size_t i = 1; i < args.size(); ++i)
		{
			std::string method = args[i];
			
			// Validate method is in the valid list
			bool isValid = false;
			for (int j = 0; validMethods[j] != NULL; ++j)
			{
				if (method == validMethods[j])
				{
					isValid = true;
					break;
				}
			}
			
			if (!isValid)
			{
				throw ArgumentException("limit_except", 
					"invalid HTTP method '" + method + "' (valid: GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH, CONNECT, TRACE)");
			}
			
			methods.push_back(method);
		}

		return new LimitExceptDirective(methods);
	}

	// Default constructor
	LimitExceptDirective::LimitExceptDirective()
		: IDirective()
	{
	}

	// Parameterized constructor
	LimitExceptDirective::LimitExceptDirective(const std::vector<std::string>& methods)
		: IDirective(), methods(methods)
	{
	}

	// Copy constructor
	LimitExceptDirective::LimitExceptDirective(const LimitExceptDirective& other)
		: IDirective(other), methods(other.methods)
	{
	}

	// Assignment operator
	LimitExceptDirective& LimitExceptDirective::operator=(const LimitExceptDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// methods is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	LimitExceptDirective::~LimitExceptDirective()
	{
	}
}
