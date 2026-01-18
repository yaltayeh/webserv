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

	// WebDAV methods for uploads
	std::set<std::string> dav_methods;
	std::string dav_access;

	bool create_full_put_path;

	// CGI/FastCGI configuration
	std::map<std::string, std::string> fastcgi_params;
	std::string fastcgi_index;

	// CGI configuration
	std::string cgi_pass;
	std::vector<std::string> cgi_extensions;

	// Upload configuration
	bool upload_enable;
	std::string upload_store;

	// Allowed HTTP methods (empty = all allowed)
	std::set<std::string> allowed_methods;

	// Access control
	bool deny_all;

	std::map<std::string, std::string> mimeTypes;

	std::string default_type;

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
		  dav_methods(),
		  dav_access("user:rw"),
		  create_full_put_path(false),
		  cgi_pass(""),
		  cgi_extensions(),
		  upload_enable(false),
		  upload_store(""),
		  allowed_methods(),
		  deny_all(false),
		  mimeTypes(),
		  default_type("text/plain")
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
		this->cgi_pass = config.cgi_pass;
		this->cgi_extensions = config.cgi_extensions;
		this->upload_enable = config.upload_enable;
		this->upload_store = config.upload_store;
		this->allowed_methods = config.allowed_methods;
		this->deny_all = config.deny_all;
		this->index = config.index;
		this->client_max_body_size = config.client_max_body_size;
		this->client_body_temp_path = config.client_body_temp_path;
		this->dav_methods = config.dav_methods;
		this->dav_access = config.dav_access;
		this->create_full_put_path = config.create_full_put_path;
		this->mimeTypes = config.mimeTypes;
		this->default_type = config.default_type;
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
			this->cgi_pass = config.cgi_pass;
			this->cgi_extensions = config.cgi_extensions;
			this->upload_enable = config.upload_enable;
			this->upload_store = config.upload_store;
			this->allowed_methods = config.allowed_methods;
			this->deny_all = config.deny_all;
			this->index = config.index;
			this->client_max_body_size = config.client_max_body_size;
			this->client_body_temp_path = config.client_body_temp_path;
			this->dav_methods = config.dav_methods;
			this->dav_access = config.dav_access;
			this->create_full_put_path = config.create_full_put_path;
			this->fastcgi_params = config.fastcgi_params;
			this->fastcgi_index = config.fastcgi_index;
			this->mimeTypes = config.mimeTypes;
			this->default_type = config.default_type;
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
