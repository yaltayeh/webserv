#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'allowed_methods' directive
 * Syntax: allowed_methods <method1> <method2> ...;
 * Example: allowed_methods GET POST;
 */
struct Config *parseAllowedMethodsDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("allowed_methods", "can only be used in http, server or location context");

	// Parse methods
	if (args.size() < 2)
		throw Directive::ArgumentException("allowed_methods", "requires at least one method");

	// Clear existing methods and add new ones
	config->allowed_methods.clear();
	for (size_t i = 1; i < args.size(); i++)
	{
		config->allowed_methods.insert(args[i]);
	}

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("allowed_methods", parseAllowedMethodsDirective);
