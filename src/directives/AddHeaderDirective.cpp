#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'add_header' directive
 * Syntax: add_header name value [always];
 * Example: add_header X-Frame-Options DENY;
 *          add_header Cache-Control "no-cache, no-store, must-revalidate";
 *          add_header X-Custom-Header "custom value" always;
 * 
 * The 'always' parameter indicates the header should be added to all responses,
 * including error responses (4xx, 5xx). Without it, headers are only added to
 * successful responses (2xx, 3xx).
 */
struct Config *parseAddHeaderDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("add_header", "can only be used in http, server or location context");

	// Parse value
	if (args.size() < 3 || args.size() > 4)
		throw Directive::ArgumentException("add_header", "requires header name and value, optionally 'always'");

	std::string headerName = args[1];
	std::string headerValue = args[2];
	bool always = false;

	// Check for 'always' flag
	if (args.size() == 4)
	{
		if (args[3] == "always")
			always = true;
		else
			throw Directive::ArgumentException("add_header", "third parameter must be 'always' if present");
	}

	// Add header to config
	CommonConfig::AddHeader header;
	header.name = headerName;
	header.value = headerValue;
	header.always = always;
	config->add_headers.push_back(header);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("add_header", parseAddHeaderDirective);
