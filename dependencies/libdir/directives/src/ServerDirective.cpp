#include "ServerDirective.hpp"
#include "HttpDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: server block allowed in HttpDirective
	DEFINE_DIRECTIVE(ServerDirective, "server", HttpDirective)

	ServerDirective* ServerDirective::initServerDirective(const std::vector<std::string>& args)
	{
		// server directive is a block directive with no arguments
		if (args.size() != 1)
		{
			throw ArgumentException("server", "takes no arguments (block directive)");
		}

		return new ServerDirective();
	}

	// Default constructor
	ServerDirective::ServerDirective()
		: IDirective()
	{
	}

	// Copy constructor
	ServerDirective::ServerDirective(const ServerDirective& other)
		: IDirective(other)
	{
	}

	// Assignment operator
	ServerDirective& ServerDirective::operator=(const ServerDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
		}
		return *this;
	}

	// Destructor
	ServerDirective::~ServerDirective()
	{
	}
}
