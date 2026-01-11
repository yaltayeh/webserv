#include "../include/Config/ServerConfig.hpp"
#include "../include/Config/RouteConfig.hpp"
#include "directives/ServerDirective.hpp"
#include "directives/ListenDirective.hpp"
#include "directives/ServerNameDirective.hpp"
#include "directives/ErrorPageDirective.hpp"
#include "directives/ClientMaxBodySizeDirective.hpp"
#include "directives/ClientBodyTempPathDirective.hpp"
#include "directives/KeepaliveTimeoutDirective.hpp"
#include "directives/ProxyRequestBufferingDirective.hpp"
#include "directives/RootDirective.hpp"
#include "directives/IndexDirective.hpp"
#include "directives/AutoindexDirective.hpp"
#include "directives/AutoindexFormatDirective.hpp"
#include "directives/AutoindexLocaltimeDirective.hpp"
#include "directives/AutoindexExactSizeDirective.hpp"
#include "directives/CgiExtensionDirective.hpp"
#include "directives/DenyDirective.hpp"
#include "directives/LocationDirective.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"

#include <cstdlib>
#include <cctype>
#include <algorithm>

// Helper functions
static size_t parseSizeToBytes(const std::string& sizeStr)
{
	if (sizeStr.empty())
		return 0;
	
	size_t digitEnd = sizeStr.length();
	for (size_t i = 0; i < sizeStr.length(); ++i)
	{
		if (!std::isdigit(sizeStr[i]))
		{
			digitEnd = i;
			break;
		}
	}
	
	std::string numStr = sizeStr.substr(0, digitEnd);
	size_t value = std::atol(numStr.c_str());
	
	if (digitEnd < sizeStr.length())
	{
		char unit = std::tolower(sizeStr[digitEnd]);
		if (unit == 'k')
			value *= 1024;
		else if (unit == 'm')
			value *= 1024 * 1024;
		else if (unit == 'g')
			value *= 1024 * 1024 * 1024;
	}
	
	return value;
}

static int parseTimeoutToSeconds(const std::string& timeStr)
{
	if (timeStr.empty())
		return 0;
	
	size_t digitEnd = timeStr.length();
	for (size_t i = 0; i < timeStr.length(); ++i)
	{
		if (!std::isdigit(timeStr[i]))
		{
			digitEnd = i;
			break;
		}
	}
	
	std::string numStr = timeStr.substr(0, digitEnd);
	int value = std::atoi(numStr.c_str());
	
	if (digitEnd < timeStr.length())
	{
		char unit = std::tolower(timeStr[digitEnd]);
		if (unit == 'm')
			value *= 60;
		else if (unit == 'h')
			value *= 3600;
	}
	
	return value;
}

static std::string autoindexFormatToString(Directive::auto_index_format format)
{
	switch (format)
	{
		case Directive::HTTP_AUTOINDEX_HTML:
			return "html";
		case Directive::HTTP_AUTOINDEX_JSON:
			return "json";
		case Directive::HTTP_AUTOINDEX_JSONP:
			return "jsonp";
		case Directive::HTTP_AUTOINDEX_XML:
			return "xml";
		default:
			return "html";
	}
}

// ============================================================================
// ServerConfig Implementation
// ============================================================================

ServerConfig::ServerConfig()
	: host("0.0.0.0"),
	  port(8080),
	  server_names(),
	  error_pages(),
	  client_max_body_size(1048576),
	  client_body_temp_path(""),
	  keepalive_timeout(75),
	  keepalive_header_timeout(0),
	  proxy_request_buffering(true),
	  root(""),
	  index_files(),
	  autoindex(false),
	  autoindex_format("html"),
	  autoindex_localtime(false),
	  autoindex_exact_size(false),
	  cgi_extensions(),
	  deny_rules(),
	  routes()
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: host(other.host),
	  port(other.port),
	  server_names(other.server_names),
	  error_pages(other.error_pages),
	  client_max_body_size(other.client_max_body_size),
	  client_body_temp_path(other.client_body_temp_path),
	  keepalive_timeout(other.keepalive_timeout),
	  keepalive_header_timeout(other.keepalive_header_timeout),
	  proxy_request_buffering(other.proxy_request_buffering),
	  root(other.root),
	  index_files(other.index_files),
	  autoindex(other.autoindex),
	  autoindex_format(other.autoindex_format),
	  autoindex_localtime(other.autoindex_localtime),
	  autoindex_exact_size(other.autoindex_exact_size),
	  cgi_extensions(other.cgi_extensions),
	  deny_rules(other.deny_rules),
	  routes(other.routes)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		host = other.host;
		port = other.port;
		server_names = other.server_names;
		error_pages = other.error_pages;
		client_max_body_size = other.client_max_body_size;
		client_body_temp_path = other.client_body_temp_path;
		keepalive_timeout = other.keepalive_timeout;
		keepalive_header_timeout = other.keepalive_header_timeout;
		proxy_request_buffering = other.proxy_request_buffering;
		root = other.root;
		index_files = other.index_files;
		autoindex = other.autoindex;
		autoindex_format = other.autoindex_format;
		autoindex_localtime = other.autoindex_localtime;
		autoindex_exact_size = other.autoindex_exact_size;
		cgi_extensions = other.cgi_extensions;
		deny_rules = other.deny_rules;
		routes = other.routes;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
}

ServerConfig ServerConfig::fromDirective(const Directive::ServerDirective* directive)
{
	ServerConfig config;
	
	// STEP 1: Process listen directive
	try {
		const Directive::ListenDirective* listen = directive->getOne<Directive::ListenDirective>();
		config.host = listen->host;
		config.port = listen->port;
	} catch (const Directive::DirectiveNotFound&) {
		config.host = "0.0.0.0";
		config.port = 80;
	}
	
	// STEP 2: Process server_name directive
	try {
		const Directive::ServerNameDirective* serverName = directive->getOne<Directive::ServerNameDirective>();
		config.server_names = serverName->names;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 3: Process error_page directives
	try {
		Directive::ListDirectives<Directive::ErrorPageDirective> errorPages = directive->getMany<Directive::ErrorPageDirective>();
		for (size_t i = 0; i < errorPages.size(); ++i)
		{
			const Directive::ErrorPageDirective* ep = errorPages[i];
			for (size_t j = 0; j < ep->codes.size(); ++j)
			{
				config.error_pages[ep->codes[j]] = ep->uri;
			}
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 4: Process client_max_body_size
	try {
		const Directive::ClientMaxBodySizeDirective* maxSize = directive->getOne<Directive::ClientMaxBodySizeDirective>();
		config.client_max_body_size = parseSizeToBytes(maxSize->size);
	} catch (const Directive::DirectiveNotFound&) {
		config.client_max_body_size = 1048576; // 1MB default
	}
	
	// STEP 5: Process client_body_temp_path
	try {
		const Directive::ClientBodyTempPathDirective* tempPath = directive->getOne<Directive::ClientBodyTempPathDirective>();
		config.client_body_temp_path = tempPath->path;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 6: Process keepalive_timeout
	try {
		const Directive::KeepaliveTimeoutDirective* keepalive = directive->getOne<Directive::KeepaliveTimeoutDirective>();
		config.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
		if (!keepalive->headerTimeout.empty())
		{
			config.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
		}
	} catch (const Directive::DirectiveNotFound&) {
		config.keepalive_timeout = 75;
	}
	
	// STEP 7: Process proxy_request_buffering
	try {
		const Directive::ProxyRequestBufferingDirective* proxyBuf = directive->getOne<Directive::ProxyRequestBufferingDirective>();
		config.proxy_request_buffering = proxyBuf->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		config.proxy_request_buffering = true;
	}
	
	// STEP 8: Process root (server-level default)
	try {
		const Directive::RootDirective* root = directive->getOne<Directive::RootDirective>();
		config.root = root->path;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 9: Process index (server-level default)
	try {
		const Directive::IndexDirective* index = directive->getOne<Directive::IndexDirective>();
		config.index_files = index->files;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 10: Process autoindex directives
	try {
		const Directive::AutoindexDirective* autoindex = directive->getOne<Directive::AutoindexDirective>();
		config.autoindex = autoindex->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		config.autoindex = false;
	}
	
	try {
		const Directive::AutoindexFormatDirective* format = directive->getOne<Directive::AutoindexFormatDirective>();
		config.autoindex_format = autoindexFormatToString(format->format);
	} catch (const Directive::DirectiveNotFound&) {
		config.autoindex_format = "html";
	}
	
	try {
		const Directive::AutoindexLocaltimeDirective* localtime = directive->getOne<Directive::AutoindexLocaltimeDirective>();
		config.autoindex_localtime = localtime->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		config.autoindex_localtime = false;
	}
	
	try {
		const Directive::AutoindexExactSizeDirective* exactSize = directive->getOne<Directive::AutoindexExactSizeDirective>();
		config.autoindex_exact_size = exactSize->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		config.autoindex_exact_size = false;
	}
	
	// STEP 11: Process cgi_extension (server-level)
	try {
		Directive::ListDirectives<Directive::CgiExtensionDirective> cgiExtensions = directive->getMany<Directive::CgiExtensionDirective>();
		for (size_t i = 0; i < cgiExtensions.size(); ++i)
		{
			const Directive::CgiExtensionDirective* ext = cgiExtensions[i];
			config.cgi_extensions.insert(config.cgi_extensions.end(),
				ext->extensions.begin(), ext->extensions.end());
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 12: Process deny directives (server-level)
	try {
		Directive::ListDirectives<Directive::DenyDirective> denyDirectives = directive->getMany<Directive::DenyDirective>();
		for (size_t i = 0; i < denyDirectives.size(); ++i)
		{
			config.deny_rules.push_back(denyDirectives[i]->address);
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// STEP 13: Process location blocks
	try {
		Directive::ListDirectives<Directive::LocationDirective> locations = directive->getMany<Directive::LocationDirective>();
		for (size_t i = 0; i < locations.size(); ++i)
		{
			RouteConfig route = RouteConfig::fromDirective(locations[i], directive);
			config.routes.push_back(route);
		}
	} catch (const Directive::DirectiveNotFound&) {
		// No locations defined - create default route
		RouteConfig defaultRoute;
		defaultRoute.path = "/";
		// Apply server-level defaults to default route
		defaultRoute.root = config.root;
		defaultRoute.index_files = config.index_files;
		defaultRoute.autoindex = config.autoindex;
		defaultRoute.autoindex_format = config.autoindex_format;
		defaultRoute.autoindex_localtime = config.autoindex_localtime;
		defaultRoute.autoindex_exact_size = config.autoindex_exact_size;
		config.routes.push_back(defaultRoute);
	}
	
	return config;
}
