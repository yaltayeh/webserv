#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "configs/CommonConfig.hpp"

/**
 * @brief Parses the 'fastcgi_param' directive
 * Syntax: fastcgi_param parameter value;
 * Example: fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
 *          fastcgi_param QUERY_STRING $query_string;
 */
struct Config *parseFastcgiParamDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("fastcgi_param", "can only be used in http, server or location context");

	// Parse value
	if (args.size() < 3)
		throw Directive::ArgumentException("fastcgi_param", "requires parameter name and value");

	// Join remaining args as value (in case value has spaces)
	std::string value;
	for (size_t i = 2; i < args.size(); i++)
	{
		if (i > 2)
			value += " ";
		value += args[i];
	}

	// Set value to config
	config->fastcgi_params[args[1]] = value;

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("fastcgi_param", parseFastcgiParamDirective);
