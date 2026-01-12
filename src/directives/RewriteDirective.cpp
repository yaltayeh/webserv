#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'rewrite' directive
 * Syntax: rewrite regex replacement [flag];
 * Example: rewrite ^/old(.*)$ /new$1 permanent;
 *          rewrite ^/temp /permanent redirect;
 * Flags: permanent (301), redirect (302)
 */
struct Config *parseRewriteDirective(const std::vector<std::string> &args, Config *parent)
{
	// Cast parent to config
	CommonConfigServerLocation *config = dynamic_cast<CommonConfigServerLocation *>(parent);
	if (!config)
		throw Directive::InvalidNesting("rewrite", "can only be used in server or location context");

	// Parse value
	if (args.size() < 3)
		throw Directive::ArgumentException("rewrite", "requires at least pattern and replacement");

	CommonConfigServerLocation::Rewrite rule;
	rule.regex = args[1];
	rule.replacement = args[2];
	rule.flag = "";

	// Check for optional flag
	if (args.size() >= 4)
		rule.flag = args[3];

	// Set value to config
	config->rewrites.push_back(rule);

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("rewrite", parseRewriteDirective);
