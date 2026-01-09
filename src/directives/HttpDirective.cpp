#include "HttpDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "GlobalDirective.hpp"

namespace Config
{
	// Register directive: http block allowed in GlobalDirective (root/main context)
	DEFINE_DIRECTIVE(HttpDirective, "http", GlobalDirective)

	HttpDirective* HttpDirective::initHttpDirective(const std::vector<std::string>& args)
	{
		// http directive is a block directive with no arguments
		if (args.size() != 1)
		{
			throw ArgumentException("http", "takes no arguments (block directive)");
		}

		return new HttpDirective();
	}

	// Default constructor
	HttpDirective::HttpDirective()
		: IDirective()
	{
	}

	// Copy constructor
	HttpDirective::HttpDirective(const HttpDirective& other)
		: IDirective(other)
	{
	}

	// Assignment operator
	HttpDirective& HttpDirective::operator=(const HttpDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
		}
		return *this;
	}

	// Destructor
	HttpDirective::~HttpDirective()
	{
	}
}
