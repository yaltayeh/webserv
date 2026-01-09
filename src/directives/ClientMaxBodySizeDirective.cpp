#include "ClientMaxBodySizeDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: client_max_body_size allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(ClientMaxBodySizeDirective, "client_max_body_size", HttpDirective, ServerDirective, LocationDirective)

	ClientMaxBodySizeDirective* ClientMaxBodySizeDirective::initClientMaxBodySizeDirective(const std::vector<std::string>& args)
	{
		// client_max_body_size requires exactly one argument (size)
		if (args.size() != 2)
		{
			throw ArgumentException("client_max_body_size", "takes exactly one argument (size)");
		}

		std::string size = args[1];
		
		// Validate size is not empty
		if (size.empty())
		{
			throw ArgumentException("client_max_body_size", "size cannot be empty");
		}

		return new ClientMaxBodySizeDirective(size);
	}

	// Default constructor
	ClientMaxBodySizeDirective::ClientMaxBodySizeDirective()
		: IDirective(), size("1m")
	{
	}

	// Parameterized constructor
	ClientMaxBodySizeDirective::ClientMaxBodySizeDirective(const std::string& size)
		: IDirective(), size(size)
	{
	}

	// Copy constructor
	ClientMaxBodySizeDirective::ClientMaxBodySizeDirective(const ClientMaxBodySizeDirective& other)
		: IDirective(other), size(other.size)
	{
	}

	// Assignment operator
	ClientMaxBodySizeDirective& ClientMaxBodySizeDirective::operator=(const ClientMaxBodySizeDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// size is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	ClientMaxBodySizeDirective::~ClientMaxBodySizeDirective()
	{
	}
}
