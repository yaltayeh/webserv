#include "ExampleDirective.hpp"
#include "GlobalDirective.hpp"
#include "DirectiveFactory.hpp"

namespace Directive
{

/**
 * Register ExampleDirective with the factory system.
 * This directive can only appear inside GlobalDirective.
 */
DEFINE_DIRECTIVE(ExampleDirective, "example", GlobalDirective)

/**
 * @brief Static initialization method - parses arguments and constructs directive
 * @param args Argument vector from configuration file
 * @return Newly constructed ExampleDirective instance
 * 
 * Arguments:
 *   args[0] = "example" (directive name)
 *   args[1] = first user argument
 *   args[2] = second user argument (optional)
 */
ExampleDirective* ExampleDirective::initExampleDirective(const std::vector<std::string>& args)
{
	// Example: Print all arguments for demonstration
	std::cout << "Parsing ExampleDirective with " << args.size() << " arguments:" << std::endl;
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "  Arg [" << i << "] = " << args[i] << std::endl;
	
	// Parse arguments (with defaults for missing ones)
	std::string arg1 = args.size() > 1 ? args[1] : "default_arg1";
	std::string arg2 = args.size() > 2 ? args[2] : "default_arg2";
	
	// Call constructor with typed parameters
	return new ExampleDirective(arg1, arg2);
}

/**
 * @brief Default constructor
 */
ExampleDirective::ExampleDirective() : arg1("default_arg1"), arg2("default_arg2") {}

/**
 * @brief Constructor with typed parameters
 * @param arg1 First argument from configuration
 * @param arg2 Second argument from configuration
 */
ExampleDirective::ExampleDirective(const std::string& arg1, const std::string& arg2)
	: arg1(arg1), arg2(arg2)
{
	std::cout << "ExampleDirective constructed with arg1='" << arg1 
	          << "' arg2='" << arg2 << "'" << std::endl;
}

/**
 * @brief Copy constructor
 * @param other ExampleDirective to copy from
 * @note Calls base class copy constructor
 */
ExampleDirective::ExampleDirective(const ExampleDirective &other) : IDirective(other)
{
	// Base class handles children copying
}

/**
 * @brief Assignment operator
 * @param other ExampleDirective to assign from
 * @return Reference to this
 */
ExampleDirective &ExampleDirective::operator=(const ExampleDirective &other)
{
	// Self-assignment check
	if (this == &other)
		return *this;
	
	// Call base class assignment
	IDirective::operator=(other);
	
	return *this;
}

/**
 * @brief Destructor
 */
ExampleDirective::~ExampleDirective() {}

} // namespace Directive
