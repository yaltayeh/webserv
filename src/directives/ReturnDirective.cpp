#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <cstdlib>

/**
 * @brief Parses the 'return' directive
 * Syntax: return code [text];
 *         return code URL;
 *         return URL;
 * Example: return 301 https://example.com;
 *          return 404;
 *          return 200 'OK';
 */
struct Config *parseReturnDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfigServerLocation *config = dynamic_cast<CommonConfigServerLocation *>(parent);
	if (!config)
		throw Directive::InvalidNesting("return", "can only be used in server or location context");

	// Parse value
	if (args.size() < 2)
		throw Directive::ArgumentException("return", "requires at least 1 argument");

	if (args.size() == 2)
	{
		// Could be: return code; or return URL;
		int code = std::atoi(args[1].c_str());
		if (code >= 100 && code <= 599)
		{
			config->return_code = code;
		}
		else
		{
			// It's a URL with implicit 302
			config->return_code = 302;
			config->return_url = args[1];
		}
	}
	else // args.size() >= 3
	{
		config->return_code = std::atoi(args[1].c_str());
		// Join remaining arguments as the URL/text
		config->return_url = args[2];
		for (size_t i = 3; i < args.size(); i++)
		{
			config->return_url += " " + args[i];
		}
	}

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("return", parseReturnDirective);
