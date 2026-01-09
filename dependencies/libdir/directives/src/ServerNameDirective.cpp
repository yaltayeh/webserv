#include "ServerNameDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "GlobalDirective.hpp"

namespace Config
{
	// Register directive: allowed in GlobalDirective (later will add ServerDirective)
	DEFINE_DIRECTIVE(ServerNameDirective, "server_name", GlobalDirective)

	ServerNameDirective* ServerNameDirective::initServerNameDirective(const std::vector<std::string>& args)
	{
		// server_name requires at least one argument (domain name)
		if (args.size() < 2)
		{
			throw ArgumentException("server_name", "requires at least one domain name");
		}

		// Collect all domain names (skip first arg which is "server_name")
		std::vector<std::string> names;
		for (size_t i = 1; i < args.size(); i++)
		{
			if (args[i].empty())
			{
				throw ArgumentException("server_name", "domain name cannot be empty");
			}
			names.push_back(args[i]);
		}

		return new ServerNameDirective(names);
	}

	// Default constructor
	ServerNameDirective::ServerNameDirective()
		: IDirective(), names()
	{
	}

	// Parameterized constructor
	ServerNameDirective::ServerNameDirective(const std::vector<std::string>& names)
		: IDirective(), names(names)
	{
	}

	// Copy constructor
	ServerNameDirective::ServerNameDirective(const ServerNameDirective& other)
		: IDirective(other), names(other.names)
	{
	}

	// Assignment operator
	ServerNameDirective& ServerNameDirective::operator=(const ServerNameDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// names is const, cannot reassign
			// This is intentional - directives are immutable after construction
		}
		return *this;
	}

	// Destructor
	ServerNameDirective::~ServerNameDirective()
	{
	}
}
