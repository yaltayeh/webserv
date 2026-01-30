#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include "Config/CommonConfig.hpp"
#include <unistd.h>  // For access()

/**
 * @brief Parses the 'cgi_pass' directive
 * Syntax: 
 *   cgi_pass <ext1> <ext2> ... <interpreter>;  (with interpreter)
 *   cgi_pass <ext1> <ext2> ...;                (binary, no interpreter)
 * 
 * Examples:
 *   cgi_pass .py .pyi /usr/bin/python3;
 *   cgi_pass .out .bin;
 */
struct Config *parseCgiPassDirective(Directive::Parser *parser, const std::vector<std::string> &args, Config *parent)
{
	(void)parser;
	// Cast parent to config
	CommonConfig *config = dynamic_cast<CommonConfig *>(parent);
	if (!config)
		throw Directive::InvalidNesting("cgi_pass", "can only be used in http, server or location context");

	// Require at least one extension
	if (args.size() < 2)
		throw Directive::ArgumentException("cgi_pass", "requires at least one extension");

	// Determine if last argument is an interpreter (doesn't start with '.')
	std::string interpreter = "";
	size_t ext_end_idx = args.size();  // Exclusive end index
	
	const std::string &last_arg = args[args.size() - 1];
	if (!last_arg.empty() && last_arg[0] != '.')
	{
		// Last arg is an interpreter
		interpreter = last_arg;
		ext_end_idx--;
		
		// Validate interpreter exists (user wants validation at parse time)
		if (access(interpreter.c_str(), X_OK) != 0)
		{
			throw Directive::ArgumentException("cgi_pass", 
				"interpreter not found or not executable: " + interpreter);
		}
	}
	
	// Must have at least one extension
	if (ext_end_idx <= 1)
	{
		throw Directive::ArgumentException("cgi_pass", 
			"requires at least one extension");
	}

	// Add each extension to the map (last one wins for duplicates)
	for (size_t i = 1; i < ext_end_idx; i++)
	{
		const std::string &ext = args[i];
		
		// Validate extension starts with '.'
		if (ext.empty() || ext[0] != '.')
		{
			throw Directive::ArgumentException("cgi_pass", 
				"extension must start with '.' (got: " + ext + ")");
		}
		
		// Add/override in map
		config->cgi_handlers[ext] = interpreter;
	}

	// Return no new config
	return NULL;
}

DEFINE_DIRECTIVE("cgi_pass", parseCgiPassDirective);
