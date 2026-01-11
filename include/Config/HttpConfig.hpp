#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP

#include <vector>
#include "ServerConfig.hpp"

// Forward declarations
namespace Directive {
	class HttpDirective;
}

/**
 * ============================================================================
 * HttpConfig - Configuration for the HTTP context
 * ============================================================================
 * 
 * WHAT THIS REPRESENTS:
 * The HTTP context is the top-level configuration container that holds all
 * server blocks and HTTP-level default settings. In NGINX-like syntax, this
 * corresponds to the "http { ... }" block.
 * 
 * EXAMPLE CONFIGURATION (NGINX-like):
 *   http {
 *       # HTTP-level defaults that apply to all servers
 *       client_max_body_size 10M;
 *       client_body_temp_path /tmp/client_body;
 *       keepalive_timeout 65;
 *       
 *       # Multiple server blocks
 *       server {
 *           listen 8080;
 *           ...
 *       }
 *       
 *       server {
 *           listen 8081;
 *           ...
 *       }
 *   }
 * 
 * CONFIGURATION HIERARCHY:
 * HTTP context (this class) contains:
 *   └── Multiple ServerConfig instances (vector<ServerConfig>)
 *         └── Each server contains multiple RouteConfig instances
 * 
 * CONTEXT INHERITANCE:
 * Directives specified at HTTP level serve as defaults for all servers.
 * Servers can override these defaults, and locations can override server
 * settings. This is handled during the fromDirective() conversion process.
 * 
 * ============================================================================
 */
class HttpConfig
{
public:
	/**
	 * List of all server configurations
	 * 
	 * IMPORTANT: Each server must have a unique host:port combination.
	 * During request handling, the appropriate server is selected based on
	 * the listening socket that received the connection.
	 * 
	 * For HTTP/1.1 virtual hosting, the Host header can be used to further
	 * select between servers listening on the same port.
	 */
	std::vector<ServerConfig> servers;
	
	/**
	 * Default constructor
	 */
	HttpConfig();
	
	/**
	 * Copy constructor
	 */
	HttpConfig(const HttpConfig& other);
	
	/**
	 * Assignment operator
	 */
	HttpConfig& operator=(const HttpConfig& other);
	
	/**
	 * Destructor
	 */
	~HttpConfig();
	
	/**
	 * Build HttpConfig from HttpDirective
	 * 
	 * This static method traverses all server directives and builds
	 * the complete HTTP configuration.
	 * 
	 * @param directive The HttpDirective to convert
	 * @return HttpConfig instance with all servers
	 */
	static HttpConfig fromDirective(const Directive::HttpDirective* directive);
};

#endif // HTTPCONFIG_HPP
