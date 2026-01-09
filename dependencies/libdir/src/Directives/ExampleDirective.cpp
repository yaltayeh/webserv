#include "Directives/ExampleDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "DirectiveFactory.hpp"

/**
 * Register ExampleDirective with the factory system.
 * This directive can only appear inside GlobalDirective.
 */
DEFINE_DIRECTIVE(ExampleDirective, "example", GlobalDirective)

/**
 * @brief Default constructor
 */
ExampleDirective::ExampleDirective() {}

/**
 * @brief Constructor with arguments - demonstrates argument access
 * @param args Vector of arguments from configuration file
 * @note This example prints all arguments for demonstration purposes
 */
ExampleDirective::ExampleDirective(const std::vector<std::string> &args)
{
	// Example: Print all arguments
	for (size_t i = 0; i < args.size(); i++)
		std::cout << "Arg [" << i << "] = " << args[i] << std::endl;
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
