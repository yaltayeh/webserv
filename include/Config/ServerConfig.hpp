#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "../../dependencies/libhttp/include/Constants.hpp"
#include "RouteConfig.hpp"

// Forward declarations
namespace Directive {
	class ServerDirective;
	class IDirective;
}

/**
 * ============================================================================
 * ServerConfig - Configuration for a single server instance
 * ============================================================================
 * 
 * WHAT THIS REPRESENTS:
 * A server block defines a listening endpoint (host:port) and all routing
 * rules, error pages, and settings for requests received on that endpoint.
 * This is equivalent to a "server" block in NGINX configuration.
 * 
 * EXAMPLE CONFIGURATION (NGINX-like):
 *   server {
 *       listen 8080;
 *       server_name localhost;
 *       
 *       error_page 404 /errors/404.html;
 *       error_page 500 502 503 504 /errors/50x.html;
 *       
 *       client_max_body_size 10M;
 *       
 *       location / {
 *           root /var/www/html;
 *           index index.html index.htm;
 *       }
 *       
 *       location /api {
 *           allowed_methods GET POST DELETE;
 *       }
 *   }
 * 
 * MULTI-SERVER ARCHITECTURE:
 * The main EventLoop can manage multiple Server instances, each with its own
 * ServerConfig. All servers share a single poll() call (42 subject requirement),
 * but each handles requests independently according to its configuration.
 * 
 * ============================================================================
 */
class ServerConfig
{
public:
	/**
	 * Hostname/IP address to bind to
	 * 
	 * Examples: 
	 * - "0.0.0.0" = all interfaces (default)
	 * - "127.0.0.1" = localhost only
	 * - "192.168.1.100" = specific interface
	 * 
	 * For multi-homed hosts, this allows binding different servers to
	 * different network interfaces.
	 */
	std::string host;
	
	/**
	 * Port number to listen on
	 * 
	 * Valid range: 1-65535
	 * Common values: 80 (HTTP), 8080 (development), 443 (HTTPS - not required)
	 * 
	 * Multiple servers can listen on different ports. If multiple servers
	 * specify the same host:port, configuration parsing should fail.
	 */
	int port;
	
	/**
	 * Server names for virtual host matching
	 * 
	 * Example: ["example.com", "www.example.com"]
	 * 
	 * In HTTP/1.1, clients send "Host" header. This field can be used for
	 * virtual hosting (multiple domains on one IP).
	 */
	std::vector<std::string> server_names;
	
	/**
	 * Custom error page paths mapped by HTTP status code
	 * 
	 * Example: 
	 *   error_pages[404] = "/errors/404.html"
	 *   error_pages[500] = "/errors/500.html"
	 * 
	 * When generating an error response, check if a custom page exists for
	 * that status code. If yes, serve the file. If no or file doesn't exist,
	 * generate a default HTML error page.
	 */
	std::map<int, std::string> error_pages;
	
	/**
	 * Maximum allowed size of client request body (in bytes)
	 * 
	 * Example: 10485760 = 10MB
	 * 
	 * If a request's Content-Length header exceeds this value, return
	 * 413 Payload Too Large before reading the body. This prevents
	 * resource exhaustion from huge uploads.
	 * 
	 * Default: 1MB (1048576 bytes)
	 */
	size_t client_max_body_size;
	
	/**
	 * Client body temporary path for storing request bodies
	 */
	std::string client_body_temp_path;
	
	/**
	 * Keepalive timeout in seconds
	 */
	int keepalive_timeout;
	
	/**
	 * Keepalive header timeout in seconds
	 */
	int keepalive_header_timeout;
	
	/**
	 * Proxy request buffering enabled
	 */
	bool proxy_request_buffering;
	
	/**
	 * Root directory (server-level default)
	 */
	std::string root;
	
	/**
	 * Index files (server-level default)
	 */
	std::vector<std::string> index_files;
	
	/**
	 * Autoindex enabled (server-level default)
	 */
	bool autoindex;
	
	/**
	 * Autoindex format (server-level default)
	 */
	std::string autoindex_format;
	
	/**
	 * Autoindex localtime (server-level default)
	 */
	bool autoindex_localtime;
	
	/**
	 * Autoindex exact size (server-level default)
	 */
	bool autoindex_exact_size;
	
	/**
	 * CGI extensions (server-level)
	 */
	std::vector<std::string> cgi_extensions;
	
	/**
	 * Deny rules (server-level)
	 */
	std::vector<std::string> deny_rules;
	
	/**
	 * List of routes/locations for this server
	 * 
	 * Order matters: During request handling, routes should be checked from
	 * most specific to least specific. The matching algorithm should use
	 * longest prefix matching.
	 * 
	 * IMPORTANT: At least one route (typically "/") should exist as a
	 * catch-all default. Configuration parsing should enforce this.
	 */
	std::vector<RouteConfig> routes;
	
	/**
	 * Default constructor - initializes safe defaults
	 */
	ServerConfig();
	
	/**
	 * Copy constructor
	 */
	ServerConfig(const ServerConfig& other);
	
	/**
	 * Assignment operator
	 */
	ServerConfig& operator=(const ServerConfig& other);
	
	/**
	 * Destructor
	 */
	~ServerConfig();
	
	/**
	 * Build ServerConfig from ServerDirective
	 * 
	 * This static method traverses the directive tree and extracts
	 * all configuration values into a ServerConfig instance.
	 * 
	 * @param directive The ServerDirective to convert
	 * @return ServerConfig instance with all values populated
	 */
	static ServerConfig fromDirective(const Directive::ServerDirective* directive);

private:
	/**
	 * Helper: Apply context-inheritable directives to RouteConfig
	 */
	static void applyContextDirectives(RouteConfig& route, 
	                                   const Directive::IDirective* context,
	                                   const Directive::ServerDirective* server);
};

#endif // SERVERCONFIG_HPP
