#include "ServerDirective.hpp"
#include "HttpDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <iostream>

// Include all directive types we need to read
#include "ListenDirective.hpp"
#include "ServerNameDirective.hpp"
#include "RootDirective.hpp"
#include "IndexDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "ClientBodyTempPathDirective.hpp"
#include "KeepaliveTimeoutDirective.hpp"
#include "ProxyRequestBufferingDirective.hpp"
#include "AutoindexDirective.hpp"
#include "AutoindexFormatDirective.hpp"
#include "AutoindexLocaltimeDirective.hpp"
#include "AutoindexExactSizeDirective.hpp"
#include "CgiExtensionDirective.hpp"
#include "DenyDirective.hpp"
#include "LocationDirective.hpp"
#include "AllowedMethodsDirective.hpp"
#include "LimitExceptDirective.hpp"
#include "AliasDirective.hpp"
#include "ReturnDirective.hpp"
#include "UploadEnableDirective.hpp"
#include "UploadStoreDirective.hpp"
#include "CgiPassDirective.hpp"
#include "CgiParamDirective.hpp"

#include <cstdlib>
#include <cctype>
#include <algorithm>

namespace Config
{
	// Register directive: server block allowed in HttpDirective
	DEFINE_DIRECTIVE(ServerDirective, "server", HttpDirective)

	// ========================================================================
	// Helper Functions for Parsing
	// ========================================================================
	
	/**
	 * Parse size string with units (k, m, g) to bytes
	 * Example: "10m" -> 10485760, "5k" -> 5120
	 */
	static size_t parseSizeToBytes(const std::string& sizeStr)
	{
		if (sizeStr.empty())
			return 0;
		
		// Find the last digit position
		size_t digitEnd = sizeStr.length();
		for (size_t i = 0; i < sizeStr.length(); ++i)
		{
			if (!std::isdigit(sizeStr[i]))
			{
				digitEnd = i;
				break;
			}
		}
		
		// Parse numeric part
		std::string numStr = sizeStr.substr(0, digitEnd);
		size_t value = std::atol(numStr.c_str());
		
		// Parse unit suffix
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
		
		// Find the last digit position
		size_t digitEnd = timeStr.length();
		for (size_t i = 0; i < timeStr.length(); ++i)
		{
			if (!std::isdigit(timeStr[i]))
			{
				digitEnd = i;
				break;
			}
		}
		
		// Parse numeric part
		std::string numStr = timeStr.substr(0, digitEnd);
		int value = std::atoi(numStr.c_str());
		
		// Parse unit suffix (default is seconds)
		if (digitEnd < timeStr.length())
		{
			char unit = std::tolower(timeStr[digitEnd]);
			if (unit == 'm')
				value *= 60;
			else if (unit == 'h')
				value *= 3600;
			// 's' or no unit = seconds (no multiplication)
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
	static std::string autoindexFormatToString(Config::auto_index_format format)
	{
		switch (format)
		{
			case HTTP_AUTOINDEX_HTML:
				return "html";
			case HTTP_AUTOINDEX_JSON:
				return "json";
			case HTTP_AUTOINDEX_JSONP:
				return "jsonp";
			case HTTP_AUTOINDEX_XML:
				return "xml";
			default:
				return "html";
		}
	}

	// ========================================================================
	// ServerDirective Methods
	// ========================================================================

	ServerDirective* ServerDirective::initServerDirective(const std::vector<std::string>& args)
	{
		// server directive is a block directive with no arguments
		if (args.size() != 1)
		{
			throw ArgumentException("server", "takes no arguments (block directive)");
		}

		return new ServerDirective();
	}

	// Default constructor
	ServerDirective::ServerDirective()
		: IDirective()
	{
	}

	// Copy constructor
	ServerDirective::ServerDirective(const ServerDirective& other)
		: IDirective(other)
	{
	}

	// Assignment operator
	ServerDirective& ServerDirective::operator=(const ServerDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
		}
		return *this;
	}

	// Destructor
	ServerDirective::~ServerDirective()
	{
	}
	
	// ========================================================================
	// Configuration Conversion
	// ========================================================================
	
	ServerConfig ServerDirective::toServerConfig() const
	{
		ServerConfig config;
		
		// ====================================================================
		// STEP 1: Process listen directive
		// ====================================================================
		try {
			const ListenDirective* listen = getOne<ListenDirective>();
			config.host = listen->host;
			config.port = listen->port;
		} catch (const DirectiveNotFound&) {
			// Use defaults
			config.host = "0.0.0.0";
			config.port = 80;
		}
		
		// ====================================================================
		// STEP 2: Process server_name directive
		// ====================================================================
		try {
			const ServerNameDirective* serverName = getOne<ServerNameDirective>();
			config.server_names = serverName->names;
		} catch (const DirectiveNotFound&) {
			// No server names defined
		}
		
		// ====================================================================
		// STEP 3: Process error_page directives (can be multiple)
		// ====================================================================
		try {
			ListDirectives<ErrorPageDirective> errorPages = getMany<ErrorPageDirective>();
			for (size_t i = 0; i < errorPages.size(); ++i)
			{
				const ErrorPageDirective* ep = errorPages[i];
				for (size_t j = 0; j < ep->codes.size(); ++j)
				{
					config.error_pages[ep->codes[j]] = ep->uri;
				}
			}
		} catch (const DirectiveNotFound&) {
			// No error pages defined
		}
		
		// ====================================================================
		// STEP 4: Process client_max_body_size
		// ====================================================================
		try {
			const ClientMaxBodySizeDirective* maxSize = getOne<ClientMaxBodySizeDirective>();
			config.client_max_body_size = parseSizeToBytes(maxSize->size);
		} catch (const DirectiveNotFound&) {
			config.client_max_body_size = 1048576; // 1MB default
		}
		
		// ====================================================================
		// STEP 5: Process client_body_temp_path
		// ====================================================================
		try {
			const ClientBodyTempPathDirective* tempPath = getOne<ClientBodyTempPathDirective>();
			config.client_body_temp_path = tempPath->path;
		} catch (const DirectiveNotFound&) {
			// No temp path defined send 413 error on large bodies
		}
		
		// ====================================================================
		// STEP 6: Process keepalive_timeout
		// ====================================================================
		try {
			const KeepaliveTimeoutDirective* keepalive = getOne<KeepaliveTimeoutDirective>();
			config.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
			if (!keepalive->headerTimeout.empty())
			{
				config.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
			}
		} catch (const DirectiveNotFound&) {
			config.keepalive_timeout = 75; // Default 75 seconds
		}
		
		// ====================================================================
		// STEP 7: Process proxy_request_buffering
		// ====================================================================
		try {
			const ProxyRequestBufferingDirective* proxyBuf = getOne<ProxyRequestBufferingDirective>();
			config.proxy_request_buffering = proxyBuf->enabled;
		} catch (const DirectiveNotFound&) {
			config.proxy_request_buffering = true; // Default on
		}
		
		// ====================================================================
		// STEP 8: Process root (server-level default)
		// ====================================================================
		try {
			const RootDirective* root = getOne<RootDirective>();
			config.root = root->path;
		} catch (const DirectiveNotFound&) {
			// No root defined
		}
		
		// ====================================================================
		// STEP 9: Process index (server-level default)
		// ====================================================================
		try {
			const IndexDirective* index = getOne<IndexDirective>();
			config.index_files = index->files;
		} catch (const DirectiveNotFound&) {
			// No index files defined
		}
		
		// ====================================================================
		// STEP 10: Process autoindex directives
		// ====================================================================
		try {
			const AutoindexDirective* autoindex = getOne<AutoindexDirective>();
			config.autoindex = autoindex->enabled;
		} catch (const DirectiveNotFound&) {
			config.autoindex = false;
		}
		
		try {
			const AutoindexFormatDirective* format = getOne<AutoindexFormatDirective>();
			config.autoindex_format = autoindexFormatToString(format->format);
		} catch (const DirectiveNotFound&) {
			config.autoindex_format = "html";
		}
		
		try {
			const AutoindexLocaltimeDirective* localtime = getOne<AutoindexLocaltimeDirective>();
			config.autoindex_localtime = localtime->enabled;
		} catch (const DirectiveNotFound&) {
			config.autoindex_localtime = false;
		}
		
		try {
			const AutoindexExactSizeDirective* exactSize = getOne<AutoindexExactSizeDirective>();
			config.autoindex_exact_size = exactSize->enabled;
		} catch (const DirectiveNotFound&) {
			config.autoindex_exact_size = false;
		}
		
		// ====================================================================
		// STEP 11: Process cgi_extension (server-level)
		// ====================================================================
		try {
			ListDirectives<CgiExtensionDirective> cgiExtensions = getMany<CgiExtensionDirective>();
			for (size_t i = 0; i < cgiExtensions.size(); ++i)
			{
				const CgiExtensionDirective* ext = cgiExtensions[i];
				config.cgi_extensions.insert(config.cgi_extensions.end(),
					ext->extensions.begin(), ext->extensions.end());
			}
		} catch (const DirectiveNotFound&) {
			// No CGI extensions defined
		}
		
		// ====================================================================
		// STEP 12: Process deny directives (server-level)
		// ====================================================================
		try {
			ListDirectives<DenyDirective> denyDirectives = getMany<DenyDirective>();
			for (size_t i = 0; i < denyDirectives.size(); ++i)
			{
				config.deny_rules.push_back(denyDirectives[i]->address);
			}
		} catch (const DirectiveNotFound&) {
			// No deny rules defined
		}
		
		// ====================================================================
		// STEP 13: Process location blocks
		// ====================================================================
		try {
			ListDirectives<LocationDirective> locations = getMany<LocationDirective>();
			for (size_t i = 0; i < locations.size(); ++i)
			{
				RouteConfig route = locationToRouteConfig(locations[i]);
				config.routes.push_back(route);
			}
		} catch (const DirectiveNotFound&) {
			// No locations defined - create default route
			RouteConfig defaultRoute;
			defaultRoute.path = "/";
			applyContextDirectives(defaultRoute, this);
			config.routes.push_back(defaultRoute);
		}
		
		return config;
	}
	
	RouteConfig ServerDirective::locationToRouteConfig(const LocationDirective* location) const
	{
		RouteConfig route;
		
		// Set path and modifier from location
		route.path = location->uri;
		route.modifier = location->modifier;
		
		// Apply context-inheritable directives from server and location
		applyContextDirectives(route, location);
		
		return route;
	}
	
	void ServerDirective::applyContextDirectives(RouteConfig& route, const IDirective* context) const
	{
		// ====================================================================
		// Process directives that can appear in server or location context
		// Location-specific values override server-level defaults
		// ====================================================================
		
		// Root
		try {
			const RootDirective* root = context->getOne<RootDirective>();
			route.root = root->path;
		} catch (const DirectiveNotFound&) {
			// Try server level if context is location
			if (context != this)
			{
				try {
					const RootDirective* root = this->getOne<RootDirective>();
					route.root = root->path;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Alias (location only)
		try {
			const AliasDirective* alias = context->getOne<AliasDirective>();
			route.alias = alias->path;
		} catch (const DirectiveNotFound&) {}
		
		// Index files
		try {
			const IndexDirective* index = context->getOne<IndexDirective>();
			route.index_files = index->files;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const IndexDirective* index = this->getOne<IndexDirective>();
					route.index_files = index->files;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Autoindex
		try {
			const AutoindexDirective* autoindex = context->getOne<AutoindexDirective>();
			route.autoindex = autoindex->enabled;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const AutoindexDirective* autoindex = this->getOne<AutoindexDirective>();
					route.autoindex = autoindex->enabled;
				} catch (const DirectiveNotFound&) {
					route.autoindex = false;
				}
			}
		}
		
		// Autoindex format
		try {
			const AutoindexFormatDirective* format = context->getOne<AutoindexFormatDirective>();
			route.autoindex_format = autoindexFormatToString(format->format);
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const AutoindexFormatDirective* format = this->getOne<AutoindexFormatDirective>();
					route.autoindex_format = autoindexFormatToString(format->format);
				} catch (const DirectiveNotFound&) {
					route.autoindex_format = "html";
				}
			}
		}
		
		// Autoindex localtime
		try {
			const AutoindexLocaltimeDirective* localtime = context->getOne<AutoindexLocaltimeDirective>();
			route.autoindex_localtime = localtime->enabled;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const AutoindexLocaltimeDirective* localtime = this->getOne<AutoindexLocaltimeDirective>();
					route.autoindex_localtime = localtime->enabled;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Autoindex exact size
		try {
			const AutoindexExactSizeDirective* exactSize = context->getOne<AutoindexExactSizeDirective>();
			route.autoindex_exact_size = exactSize->enabled;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const AutoindexExactSizeDirective* exactSize = this->getOne<AutoindexExactSizeDirective>();
					route.autoindex_exact_size = exactSize->enabled;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Allowed methods
		try {
			const AllowedMethodsDirective* allowed = context->getOne<AllowedMethodsDirective>();
			for (size_t i = 0; i < allowed->methods.size(); ++i)
			{
				libhttp::Method method = stringToMethod(allowed->methods[i]);
				if (method != libhttp::INVALID)
				{
					route.allowed_methods.push_back(method);
				}
			}
		} catch (const DirectiveNotFound&) {}
		
		// Limit except
		try {
			const LimitExceptDirective* limitExcept = context->getOne<LimitExceptDirective>();
			for (size_t i = 0; i < limitExcept->methods.size(); ++i)
			{
				libhttp::Method method = stringToMethod(limitExcept->methods[i]);
				if (method != libhttp::INVALID)
				{
					route.limit_except_methods.push_back(method);
				}
			}
		} catch (const DirectiveNotFound&) {}
		
		// Return
		try {
			const ReturnDirective* ret = context->getOne<ReturnDirective>();
			route.return_code = ret->code;
			route.return_text = ret->text;
		} catch (const DirectiveNotFound&) {}
		
		// Error pages
		try {
			ListDirectives<ErrorPageDirective> errorPages = context->getMany<ErrorPageDirective>();
			for (size_t i = 0; i < errorPages.size(); ++i)
			{
				const ErrorPageDirective* ep = errorPages[i];
				for (size_t j = 0; j < ep->codes.size(); ++j)
				{
					route.error_pages[ep->codes[j]] = ep->uri;
				}
			}
		} catch (const DirectiveNotFound&) {
			// Inherit from server level if in location context
			if (context != this)
			{
				try {
					ListDirectives<ErrorPageDirective> errorPages = this->getMany<ErrorPageDirective>();
					for (size_t i = 0; i < errorPages.size(); ++i)
					{
						const ErrorPageDirective* ep = errorPages[i];
						for (size_t j = 0; j < ep->codes.size(); ++j)
						{
							route.error_pages[ep->codes[j]] = ep->uri;
						}
					}
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Client max body size
		try {
			const ClientMaxBodySizeDirective* maxSize = context->getOne<ClientMaxBodySizeDirective>();
			route.client_max_body_size = parseSizeToBytes(maxSize->size);
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const ClientMaxBodySizeDirective* maxSize = this->getOne<ClientMaxBodySizeDirective>();
					route.client_max_body_size = parseSizeToBytes(maxSize->size);
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Client body temp path
		try {
			const ClientBodyTempPathDirective* tempPath = context->getOne<ClientBodyTempPathDirective>();
			route.client_body_temp_path = tempPath->path;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const ClientBodyTempPathDirective* tempPath = this->getOne<ClientBodyTempPathDirective>();
					route.client_body_temp_path = tempPath->path;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Keepalive timeout
		try {
			const KeepaliveTimeoutDirective* keepalive = context->getOne<KeepaliveTimeoutDirective>();
			route.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
			if (!keepalive->headerTimeout.empty())
			{
				route.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
			}
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const KeepaliveTimeoutDirective* keepalive = this->getOne<KeepaliveTimeoutDirective>();
					route.keepalive_timeout = parseTimeoutToSeconds(keepalive->timeout);
					if (!keepalive->headerTimeout.empty())
					{
						route.keepalive_header_timeout = parseTimeoutToSeconds(keepalive->headerTimeout);
					}
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Proxy request buffering
		try {
			const ProxyRequestBufferingDirective* proxyBuf = context->getOne<ProxyRequestBufferingDirective>();
			route.proxy_request_buffering = proxyBuf->enabled;
		} catch (const DirectiveNotFound&) {
			if (context != this)
			{
				try {
					const ProxyRequestBufferingDirective* proxyBuf = this->getOne<ProxyRequestBufferingDirective>();
					route.proxy_request_buffering = proxyBuf->enabled;
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// Upload enable
		try {
			const UploadEnableDirective* uploadEnable = context->getOne<UploadEnableDirective>();
			route.upload_enable = uploadEnable->enabled;
		} catch (const DirectiveNotFound&) {}
		
		// Upload store
		try {
			const UploadStoreDirective* uploadStore = context->getOne<UploadStoreDirective>();
			route.upload_store = uploadStore->path;
		} catch (const DirectiveNotFound&) {}
		
		// CGI pass
		try {
			const CgiPassDirective* cgiPass = context->getOne<CgiPassDirective>();
			route.cgi_pass = cgiPass->interpreterPath;
		} catch (const DirectiveNotFound&) {}
		
		// CGI extensions
		try {
			ListDirectives<CgiExtensionDirective> cgiExtensions = context->getMany<CgiExtensionDirective>();
			for (size_t i = 0; i < cgiExtensions.size(); ++i)
			{
				const CgiExtensionDirective* ext = cgiExtensions[i];
				route.cgi_extensions.insert(route.cgi_extensions.end(),
					ext->extensions.begin(), ext->extensions.end());
			}
		} catch (const DirectiveNotFound&) {
			// Inherit from server level if in location context
			if (context != this)
			{
				try {
					ListDirectives<CgiExtensionDirective> cgiExtensions = this->getMany<CgiExtensionDirective>();
					for (size_t i = 0; i < cgiExtensions.size(); ++i)
					{
						const CgiExtensionDirective* ext = cgiExtensions[i];
						route.cgi_extensions.insert(route.cgi_extensions.end(),
							ext->extensions.begin(), ext->extensions.end());
					}
				} catch (const DirectiveNotFound&) {}
			}
		}
		
		// CGI params (can be multiple)
		try {
			ListDirectives<CgiParamDirective> cgiParams = context->getMany<CgiParamDirective>();
			for (size_t i = 0; i < cgiParams.size(); ++i)
			{
				const CgiParamDirective* param = cgiParams[i];
				route.cgi_params[param->name] = param->value;
			}
		} catch (const DirectiveNotFound&) {}
		
		// Deny rules
		try {
			ListDirectives<DenyDirective> denyDirectives = context->getMany<DenyDirective>();
			for (size_t i = 0; i < denyDirectives.size(); ++i)
			{
				route.deny_rules.push_back(denyDirectives[i]->address);
			}
		} catch (const DirectiveNotFound&) {
			// Inherit from server level if in location context
			if (context != this)
			{
				try {
					ListDirectives<DenyDirective> denyDirectives = this->getMany<DenyDirective>();
					for (size_t i = 0; i < denyDirectives.size(); ++i)
					{
						route.deny_rules.push_back(denyDirectives[i]->address);
					}
				} catch (const DirectiveNotFound&) {}
			}
		}
	}
}
