#include "CgiPassDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: cgi_pass allowed in LocationDirective
	DEFINE_DIRECTIVE(CgiPassDirective, "cgi_pass", LocationDirective)

	CgiPassDirective* CgiPassDirective::initCgiPassDirective(const std::vector<std::string>& args)
	{
		// cgi_pass requires exactly one argument (interpreter path)
		if (args.size() != 2)
		{
			throw ArgumentException("cgi_pass", "takes exactly one argument (interpreter path)");
		}

		std::string interpreterPath = args[1];
		
		// Validate path is not empty
		if (interpreterPath.empty())
		{
			throw ArgumentException("cgi_pass", "interpreter path cannot be empty");
		}

		return new CgiPassDirective(interpreterPath);
	}

	// Default constructor
	CgiPassDirective::CgiPassDirective()
		: IDirective(), interpreterPath("")
	{
	}

	// Parameterized constructor
	CgiPassDirective::CgiPassDirective(const std::string& interpreterPath)
		: IDirective(), interpreterPath(interpreterPath)
	{
	}

	// Copy constructor
	CgiPassDirective::CgiPassDirective(const CgiPassDirective& other)
		: IDirective(other), interpreterPath(other.interpreterPath)
	{
	}

	// Assignment operator
	CgiPassDirective& CgiPassDirective::operator=(const CgiPassDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// interpreterPath is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	CgiPassDirective::~CgiPassDirective()
	{
	}
}
