#include "CgiParamDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Config
{
	// Register directive: cgi_param allowed in LocationDirective
	DEFINE_DIRECTIVE(CgiParamDirective, "cgi_param", LocationDirective)

	CgiParamDirective* CgiParamDirective::initCgiParamDirective(const std::vector<std::string>& args)
	{
		// cgi_param requires exactly two arguments: name value
		if (args.size() != 3)
		{
			throw ArgumentException("cgi_param", "takes exactly two arguments (name and value)");
		}

		std::string name = args[1];
		std::string value = args[2];
		
		// Validate name is not empty
		if (name.empty())
		{
			throw ArgumentException("cgi_param", "parameter name cannot be empty");
		}

		return new CgiParamDirective(name, value);
	}

	// Default constructor
	CgiParamDirective::CgiParamDirective()
		: IDirective(), name(""), value("")
	{
	}

	// Parameterized constructor
	CgiParamDirective::CgiParamDirective(const std::string& name, const std::string& value)
		: IDirective(), name(name), value(value)
	{
	}

	// Copy constructor
	CgiParamDirective::CgiParamDirective(const CgiParamDirective& other)
		: IDirective(other), name(other.name), value(other.value)
	{
	}

	// Assignment operator
	CgiParamDirective& CgiParamDirective::operator=(const CgiParamDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// name and value are const, cannot reassign
		}
		return *this;
	}

	// Destructor
	CgiParamDirective::~CgiParamDirective()
	{
	}
}
