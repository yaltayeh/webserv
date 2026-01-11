#include "CgiExtensionDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: cgi_extension allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(CgiExtensionDirective, "cgi_extension", HttpDirective, ServerDirective, LocationDirective)

	CgiExtensionDirective* CgiExtensionDirective::initCgiExtensionDirective(const std::vector<std::string>& args)
	{
		// cgi_extension requires at least one file extension
		if (args.size() < 2)
		{
			throw ArgumentException("cgi_extension", "requires at least one file extension");
		}

		// Collect all extensions
		std::vector<std::string> extensions;
		for (size_t i = 1; i < args.size(); i++)
		{
			std::string ext = args[i];
			
			if (ext.empty())
			{
				throw ArgumentException("cgi_extension", "extension cannot be empty");
			}
			
			// Ensure extension starts with dot
			if (ext[0] != '.')
			{
				ext = "." + ext;
			}
			
			extensions.push_back(ext);
		}

		return new CgiExtensionDirective(extensions);
	}

	// Default constructor
	CgiExtensionDirective::CgiExtensionDirective()
		: IDirective(), extensions()
	{
	}

	// Parameterized constructor
	CgiExtensionDirective::CgiExtensionDirective(const std::vector<std::string>& extensions)
		: IDirective(), extensions(extensions)
	{
	}

	// Copy constructor
	CgiExtensionDirective::CgiExtensionDirective(const CgiExtensionDirective& other)
		: IDirective(other), extensions(other.extensions)
	{
	}

	// Assignment operator
	CgiExtensionDirective& CgiExtensionDirective::operator=(const CgiExtensionDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// extensions is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	CgiExtensionDirective::~CgiExtensionDirective()
	{
	}
}
