#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <cstdlib>

static int getStatusCode(const char *code_str)
{
	char *end = NULL;
	long code = std::strtol(code_str, &end, 10);
	if (*end != '\0' || code < 100 || code > 599)
		throw Directive::ArgumentException("error_page", "invalid error code: " + std::string(code_str));
	return (code);
}

/**
 * @brief Parses the 'error_page' directive
 * Syntax: error_page code [...] uri;
 * Example: error_page 404 /404.html;
 *          error_page 500 502 503 504 /50x.html;
 */
struct Config *parseErrorPageDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("error_page", "can only be used in http, server or location context");

	// Parse value
	if (args.size() < 3)
		throw Directive::ArgumentException("error_page", "requires at least error code and uri");

	CommonConfig::ErrorPage errorPage;

	// Last argument is the URI
	errorPage.uri = args[args.size() - 1];

	// check response
	int with_response = 0;
	if (args.size() >= 4)
	{
		std::string response = errorPage.uri = args[args.size() - 2];
		if (response[0] == '=')
		{
			if (response.length() > 1)
				getStatusCode(&response.c_str()[1]);
			errorPage.response = response;
			with_response = 1;
		}
	}

	// Set value to config
	for (size_t i = 1; i < args.size() - (1 + with_response); i++)
	{
		long code = getStatusCode(args[i].c_str());
		config->error_pages[code] = errorPage;
	}

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("error_page", parseErrorPageDirective);
