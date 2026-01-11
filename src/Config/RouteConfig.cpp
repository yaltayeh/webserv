#include "../include/Config/RouteConfig.hpp"
#include "directives/LocationDirective.hpp"
#include "directives/ServerDirective.hpp"
#include "directives/RootDirective.hpp"
#include "directives/AliasDirective.hpp"
#include "directives/IndexDirective.hpp"
#include "directives/AutoindexDirective.hpp"
#include "directives/AutoindexFormatDirective.hpp"
#include "directives/AutoindexLocaltimeDirective.hpp"
#include "directives/AutoindexExactSizeDirective.hpp"
#include "directives/AllowedMethodsDirective.hpp"
#include "directives/LimitExceptDirective.hpp"
#include "directives/ReturnDirective.hpp"
#include "directives/ErrorPageDirective.hpp"
#include "directives/ClientMaxBodySizeDirective.hpp"
#include "directives/ClientBodyTempPathDirective.hpp"
#include "directives/KeepaliveTimeoutDirective.hpp"
#include "directives/ProxyRequestBufferingDirective.hpp"
#include "directives/UploadEnableDirective.hpp"
#include "directives/UploadStoreDirective.hpp"
#include "directives/CgiPassDirective.hpp"
#include "directives/CgiExtensionDirective.hpp"
#include "directives/CgiParamDirective.hpp"
#include "directives/DenyDirective.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"

#include <cstdlib>
#include <cctype>
#include <algorithm>

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Parse size string with units (k, m, g) to bytes
 * Example: "10m" -> 10485760, "5k" -> 5120
 */
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

/**
 * Parse timeout string with units (s, m, h) to seconds
 * Example: "75s" -> 75, "5m" -> 300, "1h" -> 3600
 */
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

/**
 * Convert string method name to libhttp::Method enum
 */
static libhttp::Method stringToMethod(const std::string& methodStr)
{
	std::string upper = methodStr;
	std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
	
	if (upper == "GET")
		return libhttp::GET;
	else if (upper == "POST")
		return libhttp::POST;
	else if (upper == "DELETE")
		return libhttp::DELETE;
	else
		return libhttp::INVALID;
}

/**
 * Convert autoindex format enum to string
 */
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
// RouteConfig Implementation
// ============================================================================

RouteConfig::RouteConfig()
	: path("/"),
	  modifier(""),
	  allowed_methods(),
	  limit_except_methods(),
	  root(""),
	  alias(""),
	  return_code(0),
	  return_text(""),
	  autoindex(false),
	  autoindex_format("html"),
	  autoindex_localtime(false),
	  autoindex_exact_size(false),
	  index_files(),
	  upload_enable(false),
	  upload_store(""),
	  cgi_pass(""),
	  cgi_extensions(),
	  cgi_params(),
	  deny_rules(),
	  client_body_temp_path(""),
	  client_max_body_size(0),
	  keepalive_timeout(0),
	  keepalive_header_timeout(0),
	  proxy_request_buffering(true),
	  error_pages()
{
}

RouteConfig::RouteConfig(const RouteConfig& other)
	: path(other.path),
	  modifier(other.modifier),
	  allowed_methods(other.allowed_methods),
	  limit_except_methods(other.limit_except_methods),
	  root(other.root),
	  alias(other.alias),
	  return_code(other.return_code),
	  return_text(other.return_text),
	  autoindex(other.autoindex),
	  autoindex_format(other.autoindex_format),
	  autoindex_localtime(other.autoindex_localtime),
	  autoindex_exact_size(other.autoindex_exact_size),
	  index_files(other.index_files),
	  upload_enable(other.upload_enable),
	  upload_store(other.upload_store),
	  cgi_pass(other.cgi_pass),
	  cgi_extensions(other.cgi_extensions),
	  cgi_params(other.cgi_params),
	  deny_rules(other.deny_rules),
	  client_body_temp_path(other.client_body_temp_path),
	  client_max_body_size(other.client_max_body_size),
	  keepalive_timeout(other.keepalive_timeout),
	  keepalive_header_timeout(other.keepalive_header_timeout),
	  proxy_request_buffering(other.proxy_request_buffering),
	  error_pages(other.error_pages)
{
}

RouteConfig& RouteConfig::operator=(const RouteConfig& other)
{
	if (this != &other)
	{
		path = other.path;
		modifier = other.modifier;
		allowed_methods = other.allowed_methods;
		limit_except_methods = other.limit_except_methods;
		root = other.root;
		alias = other.alias;
		return_code = other.return_code;
		return_text = other.return_text;
		autoindex = other.autoindex;
		autoindex_format = other.autoindex_format;
		autoindex_localtime = other.autoindex_localtime;
		autoindex_exact_size = other.autoindex_exact_size;
		index_files = other.index_files;
		upload_enable = other.upload_enable;
		upload_store = other.upload_store;
		cgi_pass = other.cgi_pass;
		cgi_extensions = other.cgi_extensions;
		cgi_params = other.cgi_params;
		deny_rules = other.deny_rules;
		client_body_temp_path = other.client_body_temp_path;
		client_max_body_size = other.client_max_body_size;
		keepalive_timeout = other.keepalive_timeout;
		keepalive_header_timeout = other.keepalive_header_timeout;
		proxy_request_buffering = other.proxy_request_buffering;
		error_pages = other.error_pages;
	}
	return *this;
}

RouteConfig::~RouteConfig()
{
}

RouteConfig RouteConfig::fromDirective(const Directive::LocationDirective* location,
                                        const Directive::ServerDirective* server)
{
	RouteConfig route;
	
	// Set path and modifier from location
	route.path = location->uri;
	route.modifier = location->modifier;
	
	// Process directives that can appear in location context
	// with inheritance from server context
	
	// Root
	try {
		const Directive::RootDirective* root = location->getOne<Directive::RootDirective>();
		route.root = root->path;
	} catch (const Directive::DirectiveNotFound&) {
		// Inherit from server
		try {
			const Directive::RootDirective* root = server->getOne<Directive::RootDirective>();
			route.root = root->path;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Alias (location only)
	try {
		const Directive::AliasDirective* alias = location->getOne<Directive::AliasDirective>();
		route.alias = alias->path;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Index files
	try {
		const Directive::IndexDirective* index = location->getOne<Directive::IndexDirective>();
		route.index_files = index->files;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::IndexDirective* index = server->getOne<Directive::IndexDirective>();
			route.index_files = index->files;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Autoindex
	try {
		const Directive::AutoindexDirective* autoindex = location->getOne<Directive::AutoindexDirective>();
		route.autoindex = autoindex->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::AutoindexDirective* autoindex = server->getOne<Directive::AutoindexDirective>();
			route.autoindex = autoindex->enabled;
		} catch (const Directive::DirectiveNotFound&) {
			route.autoindex = false;
		}
	}
	
	// Autoindex format
	try {
		const Directive::AutoindexFormatDirective* format = location->getOne<Directive::AutoindexFormatDirective>();
		route.autoindex_format = autoindexFormatToString(format->format);
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::AutoindexFormatDirective* format = server->getOne<Directive::AutoindexFormatDirective>();
			route.autoindex_format = autoindexFormatToString(format->format);
		} catch (const Directive::DirectiveNotFound&) {
			route.autoindex_format = "html";
		}
	}
	
	// Autoindex localtime
	try {
		const Directive::AutoindexLocaltimeDirective* localtime = location->getOne<Directive::AutoindexLocaltimeDirective>();
		route.autoindex_localtime = localtime->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::AutoindexLocaltimeDirective* localtime = server->getOne<Directive::AutoindexLocaltimeDirective>();
			route.autoindex_localtime = localtime->enabled;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Autoindex exact size
	try {
		const Directive::AutoindexExactSizeDirective* exactSize = location->getOne<Directive::AutoindexExactSizeDirective>();
		route.autoindex_exact_size = exactSize->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::AutoindexExactSizeDirective* exactSize = server->getOne<Directive::AutoindexExactSizeDirective>();
			route.autoindex_exact_size = exactSize->enabled;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Allowed methods
	try {
		const Directive::AllowedMethodsDirective* allowed = location->getOne<Directive::AllowedMethodsDirective>();
		for (size_t i = 0; i < allowed->methods.size(); ++i)
		{
			libhttp::Method method = stringToMethod(allowed->methods[i]);
			if (method != libhttp::INVALID)
			{
				route.allowed_methods.push_back(method);
			}
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Limit except
	try {
		const Directive::LimitExceptDirective* limitExcept = location->getOne<Directive::LimitExceptDirective>();
		for (size_t i = 0; i < limitExcept->methods.size(); ++i)
		{
			libhttp::Method method = stringToMethod(limitExcept->methods[i]);
			if (method != libhttp::INVALID)
			{
				route.limit_except_methods.push_back(method);
			}
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Return
	try {
		const Directive::ReturnDirective* ret = location->getOne<Directive::ReturnDirective>();
		route.return_code = ret->code;
		route.return_text = ret->text;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Error pages
	try {
		Directive::ListDirectives<Directive::ErrorPageDirective> errorPages = location->getMany<Directive::ErrorPageDirective>();
		for (size_t i = 0; i < errorPages.size(); ++i)
		{
			const Directive::ErrorPageDirective* ep = errorPages[i];
			for (size_t j = 0; j < ep->codes.size(); ++j)
			{
				route.error_pages[ep->codes[j]] = ep->uri;
			}
		}
	} catch (const Directive::DirectiveNotFound&) {
		// Inherit from server
		try {
			Directive::ListDirectives<Directive::ErrorPageDirective> errorPages = server->getMany<Directive::ErrorPageDirective>();
			for (size_t i = 0; i < errorPages.size(); ++i)
			{
				const Directive::ErrorPageDirective* ep = errorPages[i];
				for (size_t j = 0; j < ep->codes.size(); ++j)
				{
					route.error_pages[ep->codes[j]] = ep->uri;
				}
			}
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Client max body size
	try {
		const Directive::ClientMaxBodySizeDirective* maxSize = location->getOne<Directive::ClientMaxBodySizeDirective>();
		route.client_max_body_size = parseSizeToBytes(maxSize->size);
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::ClientMaxBodySizeDirective* maxSize = server->getOne<Directive::ClientMaxBodySizeDirective>();
			route.client_max_body_size = parseSizeToBytes(maxSize->size);
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Client body temp path
	try {
		const Directive::ClientBodyTempPathDirective* tempPath = location->getOne<Directive::ClientBodyTempPathDirective>();
		route.client_body_temp_path = tempPath->path;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::ClientBodyTempPathDirective* tempPath = server->getOne<Directive::ClientBodyTempPathDirective>();
			route.client_body_temp_path = tempPath->path;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Keepalive timeout
	try {
		const Directive::KeepaliveTimeoutDirective* keepalive = location->getOne<Directive::KeepaliveTimeoutDirective>();
		route.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
		if (!keepalive->headerTimeout.empty())
		{
			route.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
		}
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::KeepaliveTimeoutDirective* keepalive = server->getOne<Directive::KeepaliveTimeoutDirective>();
			route.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
			if (!keepalive->headerTimeout.empty())
			{
				route.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
			}
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Proxy request buffering
	try {
		const Directive::ProxyRequestBufferingDirective* proxyBuf = location->getOne<Directive::ProxyRequestBufferingDirective>();
		route.proxy_request_buffering = proxyBuf->enabled;
	} catch (const Directive::DirectiveNotFound&) {
		try {
			const Directive::ProxyRequestBufferingDirective* proxyBuf = server->getOne<Directive::ProxyRequestBufferingDirective>();
			route.proxy_request_buffering = proxyBuf->enabled;
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// Upload enable
	try {
		const Directive::UploadEnableDirective* uploadEnable = location->getOne<Directive::UploadEnableDirective>();
		route.upload_enable = uploadEnable->enabled;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Upload store
	try {
		const Directive::UploadStoreDirective* uploadStore = location->getOne<Directive::UploadStoreDirective>();
		route.upload_store = uploadStore->path;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// CGI pass
	try {
		const Directive::CgiPassDirective* cgiPass = location->getOne<Directive::CgiPassDirective>();
		route.cgi_pass = cgiPass->interpreterPath;
	} catch (const Directive::DirectiveNotFound&) {}
	
	// CGI extensions
	try {
		Directive::ListDirectives<Directive::CgiExtensionDirective> cgiExtensions = location->getMany<Directive::CgiExtensionDirective>();
		for (size_t i = 0; i < cgiExtensions.size(); ++i)
		{
			const Directive::CgiExtensionDirective* ext = cgiExtensions[i];
			route.cgi_extensions.insert(route.cgi_extensions.end(),
				ext->extensions.begin(), ext->extensions.end());
		}
	} catch (const Directive::DirectiveNotFound&) {
		// Inherit from server
		try {
			Directive::ListDirectives<Directive::CgiExtensionDirective> cgiExtensions = server->getMany<Directive::CgiExtensionDirective>();
			for (size_t i = 0; i < cgiExtensions.size(); ++i)
			{
				const Directive::CgiExtensionDirective* ext = cgiExtensions[i];
				route.cgi_extensions.insert(route.cgi_extensions.end(),
					ext->extensions.begin(), ext->extensions.end());
			}
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	// CGI params
	try {
		Directive::ListDirectives<Directive::CgiParamDirective> cgiParams = location->getMany<Directive::CgiParamDirective>();
		for (size_t i = 0; i < cgiParams.size(); ++i)
		{
			const Directive::CgiParamDirective* param = cgiParams[i];
			route.cgi_params[param->name] = param->value;
		}
	} catch (const Directive::DirectiveNotFound&) {}
	
	// Deny rules
	try {
		Directive::ListDirectives<Directive::DenyDirective> denyDirectives = location->getMany<Directive::DenyDirective>();
		for (size_t i = 0; i < denyDirectives.size(); ++i)
		{
			route.deny_rules.push_back(denyDirectives[i]->address);
		}
	} catch (const Directive::DirectiveNotFound&) {
		// Inherit from server
		try {
			Directive::ListDirectives<Directive::DenyDirective> denyDirectives = server->getMany<Directive::DenyDirective>();
			for (size_t i = 0; i < denyDirectives.size(); ++i)
			{
				route.deny_rules.push_back(denyDirectives[i]->address);
			}
		} catch (const Directive::DirectiveNotFound&) {}
	}
	
	return route;
}
