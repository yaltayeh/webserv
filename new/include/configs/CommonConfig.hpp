#pragma once

#include <string>
#include <map>
#include <set>
#include "Config.hpp"

struct CommonConfig : Config
{
	bool autoindex;
	bool autoindex_exact_size;
	std::string autoindex_format;

	// Error page mappings: error_code -> uri
	struct ErrorPage
	{
		std::string response;
		std::string uri;
	};
	std::map<int, ErrorPage> error_pages;

	// Root directory for serving files
	std::string root;

	// Default index files
	std::vector<std::string> index;

	// Client body size override (inherits from http if not set)
	size_t client_max_body_size;

	// Temporary path for client body
	std::string client_body_temp_path;

	// WebDAV methods for uploads
	std::set<std::string> dav_methods;
	std::string dav_access;

	bool create_full_put_path;

	// CGI/FastCGI configuration
	std::map<std::string, std::string> fastcgi_params;
	std::string fastcgi_index;

	CommonConfig(Config *parent)
		: Config(parent),
		autoindex(false),
		autoindex_exact_size(true),
		autoindex_format("html"),
		error_pages(),
		root("html"),
		index(),
		client_max_body_size(1024 * 1024),
		client_body_temp_path("client_body_temp"),
		dav_methods(),
		dav_access("user:rw"),
		create_full_put_path(false)
	{
		index.push_back("index.html");
	}
};

struct CommonConfigServerLocation : CommonConfig
{
	std::vector<struct LocationConfig *> locations;

	// HTTP redirection
	bool is_return;
	int return_code;
	std::string return_url;

	struct Rewrite
	{
		std::string regex;
		std::string replacement;
		std::string flag;
	} ;
	std::vector<Rewrite> rewrites;

	CommonConfigServerLocation(Config *parent)
		: CommonConfig(parent),

		  is_return(false)
	{
	}
};
