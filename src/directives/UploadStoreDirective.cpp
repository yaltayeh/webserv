#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"

/**
 * @brief Parses the 'upload_store' directive
 * Syntax: upload_store <path>;
 * Example: upload_store /var/www/uploads;
 */
struct Config *parseUploadStoreDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("upload_store", "can only be used in http, server or location context");

	// Parse path
	if (args.size() != 2)
		throw Directive::ArgumentException("upload_store", "requires exactly one argument (path)");

	config->upload_store = args[1];

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("upload_store", parseUploadStoreDirective);
