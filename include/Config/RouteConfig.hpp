#ifndef ROUTECONFIG_HPP
#define ROUTECONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "../../dependencies/libhttp/include/Constants.hpp"

// Forward declarations
namespace Directive {
	class LocationDirective;
	class IDirective;
	class ServerDirective;
}

/**
 * ============================================================================
 * RouteConfig - Configuration for a single route/location
 * ============================================================================
 * 
 * WHAT THIS REPRESENTS:
 * A route defines how the server handles requests matching a specific URI path.
 * This is equivalent to a "location" block in NGINX configuration.
 * 
 * EXAMPLE CONFIGURATION (NGINX-like):
 *   location /api {
 *       allowed_methods GET POST;
 *       root /var/www/api;
 *   }
 * 
 *   location /upload {
 *       allowed_methods POST;
 *       upload_store /tmp/uploads;
 *       client_max_body_size 10M;
 *   }
 * 
 *   location ~ \.php$ {
 *       cgi_executor /usr/bin/php-cgi;
 *   }
 * 
 * MATCHING ALGORITHM:
 * Routes will be matched using longest prefix matching (like NGINX). For
 * example, if routes exist for "/" and "/api", a request to "/api/users"
 * will match "/api" (more specific).
 * 
 * ============================================================================
 */
class RouteConfig
{
public:
	/**
	 * Path prefix to match against request URI
	 * 
	 * Examples: "/", "/api", "/static", "/upload"
	 * 
	 * NOTE: Use "/" for catch-all default route. More specific routes should
	 * be checked first during matching.
	 */
	std::string path;
	
	/**
	 * Location modifier for matching behavior
	 * 
	 * Examples: "=" for exact match, "" for prefix match, "@" for named location
	 */
	std::string modifier;
	
	/**
	 * HTTP methods allowed for this route
	 * 
	 * Example: [GET, POST] for API endpoints, [GET] for static files
	 * 
	 * If empty, all methods are allowed. If specified, requests with methods
	 * not in this list should return 405 Method Not Allowed.
	 */
	std::vector<libhttp::Method> allowed_methods;
	
	/**
	 * HTTP methods for limit_except directive (inverted logic)
	 * 
	 * These are the ONLY methods allowed when limit_except is used.
	 * All other methods return 403 Forbidden.
	 */
	std::vector<libhttp::Method> limit_except_methods;
	
	/**
	 * Root directory for file serving
	 * 
	 * Example: "/var/www/html" - files will be served from this directory
	 * combined with the request URI path.
	 * 
	 * Request: GET /images/logo.png
	 * File path: <root>/images/logo.png = /var/www/html/images/logo.png
	 */
	std::string root;
	
	/**
	 * Alias path - alternative to root for location replacement
	 * 
	 * Unlike root which appends location to path, alias replaces the location.
	 * Example: location /images { alias /data/pics; }
	 * Request: /images/logo.png -> /data/pics/logo.png
	 */
	std::string alias;
	
	/**
	 * Return directive - HTTP status code
	 * 
	 * If non-zero, return this status code immediately with optional text/URL.
	 */
	int return_code;
	
	/**
	 * Return directive - text/URL to return
	 * 
	 * Can be redirect URL (3xx codes) or response text.
	 */
	std::string return_text;
	
	/**
	 * Enable directory listing (auto-index)
	 * 
	 * When true: If request URI is a directory and no index file exists,
	 * generate an HTML page listing directory contents.
	 * 
	 * When false: Return 403 Forbidden for directory requests without index.
	 */
	bool autoindex;
	
	/**
	 * Autoindex format: html, json, xml, jsonp
	 */
	std::string autoindex_format;
	
	/**
	 * Autoindex - use local time instead of UTC
	 */
	bool autoindex_localtime;
	
	/**
	 * Autoindex - show exact file sizes in bytes
	 */
	bool autoindex_exact_size;
	
	/**
	 * Default index files to serve for directory requests
	 * 
	 * Example: ["index.html", "index.htm", "default.html"]
	 * 
	 * When a request URI resolves to a directory, check these files in order.
	 * Serve the first one that exists. If none exist and autoindex
	 * is false, return 403 Forbidden.
	 */
	std::vector<std::string> index_files;
	
	/**
	 * Enable file uploads for this location
	 */
	bool upload_enable;
	
	/**
	 * Directory for storing uploaded files (for POST requests)
	 * 
	 * Example: "/tmp/uploads"
	 * 
	 * When handling POST requests with multipart/form-data, uploaded files
	 * should be written to this directory. If empty, file uploads are not
	 * allowed for this route.
	 */
	std::string upload_store;
	
	/**
	 * CGI executor path (for dynamic script execution)
	 * 
	 * Example: "/usr/bin/php-cgi" for .php files
	 *          "/usr/bin/python3" for .py files
	 * 
	 * When non-empty and request matches a file with appropriate extension,
	 * fork and execute this program with the script path, passing HTTP
	 * request data via environment variables and stdin (for POST body).
	 * 
	 * IMPORTANT: Must un-chunk Transfer-Encoding: chunked requests before
	 * passing to CGI (CGI expects Content-Length).
	 */
	std::string cgi_pass;
	
	/**
	 * CGI file extensions
	 * 
	 * Example: [".php", ".py", ".pl"]
	 * 
	 * Only execute CGI if request URI ends with one of these extensions.
	 */
	std::vector<std::string> cgi_extensions;
	
	/**
	 * CGI environment variables (parameter name -> value)
	 * 
	 * Additional environment variables to pass to CGI scripts.
	 */
	std::map<std::string, std::string> cgi_params;
	
	/**
	 * Access control - denied addresses
	 * 
	 * List of IP addresses, CIDR blocks, or "all" to deny access.
	 */
	std::vector<std::string> deny_rules;
	
	/**
	 * Client body temp path for this location
	 */
	std::string client_body_temp_path;
	
	/**
	 * Maximum allowed size of client request body (in bytes)
	 * 
	 * 0 means use server default.
	 */
	size_t client_max_body_size;
	
	/**
	 * Keepalive timeout in seconds (0 = use server default)
	 */
	int keepalive_timeout;
	
	/**
	 * Keepalive header timeout in seconds
	 */
	int keepalive_header_timeout;
	
	/**
	 * Proxy request buffering (on/off)
	 */
	bool proxy_request_buffering;
	
	/**
	 * Custom error pages for this location
	 */
	std::map<int, std::string> error_pages;
	
	/**
	 * Default constructor - initializes safe defaults
	 */
	RouteConfig();
	
	/**
	 * Copy constructor
	 */
	RouteConfig(const RouteConfig& other);
	
	/**
	 * Assignment operator
	 */
	RouteConfig& operator=(const RouteConfig& other);
	
	/**
	 * Destructor
	 */
	~RouteConfig();
	
	/**
	 * Build RouteConfig from LocationDirective
	 * 
	 * This static method traverses the directive and extracts
	 * all configuration values, with context inheritance from server.
	 * 
	 * @param location The LocationDirective to convert
	 * @param server The parent ServerDirective for inheritance
	 * @return RouteConfig instance with all values populated
	 */
	static RouteConfig fromDirective(const Directive::LocationDirective* location,
	                                  const Directive::ServerDirective* server);
};

#endif // ROUTECONFIG_HPP
