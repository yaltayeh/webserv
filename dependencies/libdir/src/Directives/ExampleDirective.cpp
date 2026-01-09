#include "Directives/ExampleDirective.hpp"
#include "Directives/GlobalDirective.hpp"

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
 * @brief Destructor
 */
ExampleDirective::~ExampleDirective() {}
