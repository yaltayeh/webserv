/**
 * ============================================================================
 * config.hpp - Server and Route Configuration Structures
 * ============================================================================
 * 
 * WHAT THIS FILE CONTAINS:
 * - ServerConfig: Configuration for a single server instance (host, port, 
 *   error pages, body size limits, routes)
 * - RouteConfig: Configuration for a specific route/location within a server
 *   (path matching, allowed methods, file serving settings, CGI settings)
 * 
 * WHY THESE STRUCTURES EXIST:
 * The 42 School subject requires supporting multiple server blocks (like NGINX),
 * where each server can listen on different host:port pairs and have different
 * routing rules, error pages, and CGI configurations. These POD (Plain Old Data)
 * structures store the parsed configuration from the config file.
 * 
 * DESIGN DECISIONS:
 * - Flat struct design (no inheritance) for C++98 simplicity
 * - std::vector for dynamic arrays (routes, allowed methods, index files)
 * - std::map for key-value pairs (error pages: status code -> file path)
 * - std::string for text data (paths, hostnames, redirects)
 * 
 * FUTURE USAGE:
 * These structures will be populated by a configuration file parser (not yet
 * implemented) and stored in Server class instances. During request handling,
 * the ConnectionManager will use these configs to determine routing, method
 * validation, file serving, and error page generation.
 * 
 * ============================================================================
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "../dependencies/libhttp/include/Constants.hpp"

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
struct RouteConfig
{
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
    RouteConfig()
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
        /* All fields initialized in member initializer list */
    }
};

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
struct ServerConfig
{
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
     * 
     * NOTE: These defaults are suitable for development/testing. Production
     * deployments should always use explicit configuration files.
     */
    ServerConfig()
        : host("0.0.0.0"),
          port(8080),
          server_names(),
          error_pages(),
          client_max_body_size(1048576),  /* 1MB default */
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
        /* All fields initialized in member initializer list */
    }
};

#endif /* CONFIG_HPP */
