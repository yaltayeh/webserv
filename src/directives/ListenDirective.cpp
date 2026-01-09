#include "ListenDirective.hpp"
#include "ServerDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <cstdlib>

namespace Config
{
	// Register directive: listen allowed in ServerDirective
	DEFINE_DIRECTIVE(ListenDirective, "listen", ServerDirective)

	ListenDirective* ListenDirective::initListenDirective(const std::vector<std::string>& args)
	{
		// listen requires at least port argument
		if (args.size() < 2)
		{
			throw ArgumentException("listen", "requires at least port number");
		}

		// Parse port (can be just port number or host:port)
		int port = 0;
		std::string host = "0.0.0.0";
		
		// Check if arg contains ':' for host:port format
		std::string firstArg = args[1];
		size_t colonPos = firstArg.find(':');
		
		if (colonPos != std::string::npos)
		{
			// Format: host:port
			host = firstArg.substr(0, colonPos);
			std::string portStr = firstArg.substr(colonPos + 1);
			port = std::atoi(portStr.c_str());
		}
		else
		{
			// Just port number
			port = std::atoi(firstArg.c_str());
			
			// Optional host as second argument
			if (args.size() > 2)
			{
				host = args[2];
			}
		}
		
		// Validate port range
		if (port < 1 || port > 65535)
		{
			throw ArgumentException("listen", "port must be between 1-65535");
		}

		return new ListenDirective(port, host);
	}

	// Default constructor
	ListenDirective::ListenDirective()
		: IDirective(), port(80), host("0.0.0.0")
	{
	}

	// Parameterized constructor
	ListenDirective::ListenDirective(int port, const std::string& host)
		: IDirective(), port(port), host(host)
	{
	}

	// Copy constructor
	ListenDirective::ListenDirective(const ListenDirective& other)
		: IDirective(other), port(other.port), host(other.host)
	{
	}

	// Assignment operator
	ListenDirective& ListenDirective::operator=(const ListenDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// port and host are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	ListenDirective::~ListenDirective()
	{
	}
}
