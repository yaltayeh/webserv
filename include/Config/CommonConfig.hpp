#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Config.hpp"

// Forward declaration to break circular dependency
struct LocationConfig;

struct CommonConfig : Config
{
	bool autoindex;
	bool autoindex_exact_size;
	std::string autoindex_format;

	// Error page mappings: error_code -> uri
	struct ErrorPage
	{
		bool customRes;
		int response;
		std::string uri;

		ErrorPage()
			: customRes(false),
			  response(-1),
			  uri() {}
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

	// CGI configuration
	std::map<std::string, std::string> cgi_handlers;  // extension -> interpreter path

	// Allowed HTTP methods (empty = all allowed)
	std::set<std::string> allowed_methods;

	// Access control
	bool deny_all;

	std::map<std::string, std::string> mimeTypes;

	std::string default_type;

	// Custom headers to add to responses
	struct AddHeader
	{
		std::string name;
		std::string value;
		bool always;  // Add to error responses too

		AddHeader()
			: name(),
			  value(),
			  always(false) {}
	};
	std::vector<AddHeader> add_headers;

	CommonConfig()
		: Config(),
		  autoindex(false),
		  autoindex_exact_size(true),
		  autoindex_format("html"),
		  error_pages(),
		  root("html"),
		  index(),
		  client_max_body_size(1024 * 1024),
		  client_body_temp_path("client_body_temp"),
		  cgi_handlers(),
		  allowed_methods(),
		  deny_all(false),
		  mimeTypes(),
		  default_type("text/plain"),
		  add_headers()
	{
		index.push_back("index.html");
		mimeTypes["html"] = "text/html";
		mimeTypes["gif"] = "image/gif";
		mimeTypes["jpg"] = "image/jpeg";
	}

	CommonConfig(const CommonConfig &config)
		: Config(config)
	{
		this->autoindex = config.autoindex;
		this->autoindex_exact_size = config.autoindex_exact_size;
		this->autoindex_format = config.autoindex_format;
		this->error_pages = config.error_pages;
		this->root = config.root;
		this->cgi_handlers = config.cgi_handlers;
		this->allowed_methods = config.allowed_methods;
		this->deny_all = config.deny_all;
		this->index = config.index;
		this->client_max_body_size = config.client_max_body_size;
		this->client_body_temp_path = config.client_body_temp_path;
		this->mimeTypes = config.mimeTypes;
		this->default_type = config.default_type;
		this->add_headers = config.add_headers;
	}

	CommonConfig &operator=(const CommonConfig &config)
	{
		if (this != &config)
		{
			Config::operator=(config);
			this->autoindex = config.autoindex;
			this->autoindex_exact_size = config.autoindex_exact_size;
			this->autoindex_format = config.autoindex_format;
			this->error_pages = config.error_pages;
			this->root = config.root;
			this->cgi_handlers = config.cgi_handlers;
			this->allowed_methods = config.allowed_methods;
			this->deny_all = config.deny_all;
			this->index = config.index;
			this->client_max_body_size = config.client_max_body_size;
			this->client_body_temp_path = config.client_body_temp_path;
			this->mimeTypes = config.mimeTypes;
			this->default_type = config.default_type;
			this->add_headers = config.add_headers;
		}
		return *this;
	}

	~CommonConfig() {}
};

struct CommonConfigServerLocation : CommonConfig
{
	std::vector<struct LocationConfig> locations;

	// HTTP redirection
	bool is_return;
	int return_code;
	std::string return_url;

	struct Rewrite
	{
		std::string regex;
		std::string replacement;
		std::string flag;
	};
	std::vector<Rewrite> rewrites;

	CommonConfigServerLocation()
		: CommonConfig(),
		  is_return(false) {}

	CommonConfigServerLocation(const CommonConfigServerLocation &other)
		: CommonConfig(other)
	{
		this->locations = other.locations;
		this->is_return = other.is_return;
		this->return_code = other.return_code;
		this->return_url = other.return_url;
		this->rewrites = other.rewrites;
	}

	CommonConfigServerLocation &operator=(const CommonConfigServerLocation &other)
	{
		if (this != &other)
		{
			CommonConfig::operator=(other);
			this->locations = other.locations;
			this->is_return = other.is_return;
			this->return_code = other.return_code;
			this->return_url = other.return_url;
			this->rewrites = other.rewrites;
		}
		return *this;
	}

	~CommonConfigServerLocation() {}
};

// Include LocationConfig after CommonConfigServerLocation is defined
// to break circular dependency
#include "LocationConfig.hpp"
