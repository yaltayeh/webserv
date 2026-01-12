/**
 * RequestRouter.hpp - HTTP Request Routing and Response Generation
 * 
 * WHAT THIS CLASS DOES:
 * Handles the complete request-response cycle by routing HTTP requests to the
 * appropriate handler (static file, CGI script, redirect, error page) based on
 * the server configuration and request URI.
 * 
 * RESPONSIBILITIES:
 * - Match request URI against server location blocks
 * - Serve static files from the filesystem
 * - Execute CGI scripts and return their output
 * - Handle HTTP redirects (return directive)
 * - Generate error responses (403, 404, 405, 413, 500, etc.)
 * - Validate allowed methods per location
 * - Check access control (deny directive)
 * 
 * DESIGN:
 * - Stateless: Each route() call is independent
 * - Uses ServerConfig for routing rules
 * - Delegates to libhttp::Response for response construction
 * - Delegates to CGI class for script execution
 * 
 * USAGE:
 * ```cpp
 * RequestRouter router;
 * libhttp::Response response = router.route(request, server_config);
 * std::string response_str = response.toString();
 * ```
 */

#ifndef REQUEST_ROUTER_HPP
#define REQUEST_ROUTER_HPP

#include "Config/ServerConfig.hpp"
#include "../dependencies/libhttp/include/Request.hpp"
#include "../dependencies/libhttp/include/Response.hpp"
#include <string>

class RequestRouter
{
public:
    RequestRouter();
    ~RequestRouter();
    
    /**
     * route() - Routes an HTTP request and generates the appropriate response
     * 
     * This is the main entry point for request processing. It:
     * 1. Matches the request URI against location blocks
     * 2. Validates the request method is allowed
     * 3. Checks access control (deny directive)
     * 4. Handles redirects (return directive)
     * 5. Serves static files or executes CGI scripts
     * 6. Generates error pages for failures
     * 
     * PARAMETERS:
     * - request: The parsed HTTP request
     * - config: The server configuration (routes, root, error pages, etc.)
     * 
     * RETURNS:
     * - A complete libhttp::Response object ready to be serialized
     */
    libhttp::Response route(const libhttp::Request& request, const ServerConfig& config);

private:
    // Disable copy (C++98 style)
    RequestRouter(const RequestRouter& other);
    RequestRouter& operator=(const RequestRouter& other);
    
    // ===== ROUTING LOGIC =====
    
    /**
     * findMatchingLocation() - Finds the best matching location block for a URI
     * 
     * Implements longest-prefix matching:
     * - /images/cat.jpg matches both "/" and "/images"
     * - "/images" wins because it's more specific (longer prefix)
     * 
     * RETURNS:
     * - Pointer to matching LocationConfig, or NULL if no match
     */
    const LocationConfig* findMatchingLocation(const std::string& uri,
                                               const ServerConfig& config) const;
    
    /**
     * getEffectiveConfig() - Merges server and location config
     * 
     * Location blocks inherit settings from server block but can override them.
     * This function returns the effective configuration for a matched location.
     * 
     * PARAMETERS:
     * - server_config: Base configuration from server block
     * - location: Matched location block (can be NULL for server-level config)
     * 
     * RETURNS:
     * - Effective CommonConfigServerLocation with merged settings
     */
    const CommonConfigServerLocation* getEffectiveConfig(
        const ServerConfig& server_config,
        const LocationConfig* location) const;
    
    // ===== HANDLER FUNCTIONS =====
    
    /**
     * handleStaticFile() - Serves a file from the filesystem
     * 
     * Handles:
     * - Reading file from disk
     * - Setting Content-Type based on file extension
     * - Setting Content-Length
     * - Directory index (index.html, etc.)
     * - Autoindex (directory listing) if enabled
     * - 404 if file not found
     * - 403 if not readable
     */
    libhttp::Response handleStaticFile(const std::string& uri,
                                       const CommonConfigServerLocation* config);
    
    /**
     * handleCGI() - Executes a CGI script and returns its output
     * 
     * Handles:
     * - Determining script path from URI
     * - Setting up CGI environment variables
     * - Executing script with appropriate interpreter
     * - Parsing CGI output (headers + body)
     * - Timeout handling
     * - 500 if CGI execution fails
     */
    libhttp::Response handleCGI(const libhttp::Request& request,
                                const std::string& uri,
                                const CommonConfigServerLocation* config);
    
    /**
     * handleRedirect() - Generates HTTP redirect response
     * 
     * Handles return directive:
     * - return 301 /new-location;
     * - return 302 https://example.com;
     */
    libhttp::Response handleRedirect(int status_code, const std::string& url);
    
    /**
     * handleAutoindex() - Generates directory listing
     * 
     * Lists files and subdirectories in HTML format when:
     * - No index file found
     * - autoindex is enabled
     */
    libhttp::Response handleAutoindex(const std::string& directory_path,
                                      const std::string& uri);
    
    // ===== ERROR HANDLING =====
    
    /**
     * generateErrorResponse() - Creates error response with custom error page
     * 
     * Checks for custom error_page configuration first, falls back to default.
     * 
     * PARAMETERS:
     * - status_code: HTTP status code (400, 403, 404, 500, etc.)
     * - config: Configuration that may contain custom error_page mappings
     * 
     * RETURNS:
     * - Complete error response (custom page or default HTML)
     */
    libhttp::Response generateErrorResponse(int status_code,
                                            const CommonConfigServerLocation* config);
    
    /**
     * generateDefaultErrorPage() - Creates default HTML error page
     * 
     * Used when no custom error_page is configured.
     */
    libhttp::Response generateDefaultErrorPage(int status_code);
    
    // ===== VALIDATION =====
    
    /**
     * isMethodAllowed() - Checks if HTTP method is allowed for this location
     * 
     * RETURNS:
     * - true if method is allowed (or no restrictions configured)
     * - false if method is explicitly denied
     */
    bool isMethodAllowed(libhttp::Method method,
                        const CommonConfigServerLocation* config) const;
    
    /**
     * isAccessDenied() - Checks if access is denied (deny directive)
     * 
     * RETURNS:
     * - true if deny_all is enabled
     * - false otherwise
     */
    bool isAccessDenied(const CommonConfigServerLocation* config) const;
    
    /**
     * exceedsMaxBodySize() - Checks if request body exceeds limit
     * 
     * RETURNS:
     * - true if Content-Length exceeds client_max_body_size
     * - false otherwise
     */
    bool exceedsMaxBodySize(const libhttp::Request& request,
                           const CommonConfigServerLocation* config) const;
    
    // ===== UTILITIES =====
    
    /**
     * resolvePath() - Resolves URI to filesystem path using root or alias
     * 
     * Handles:
     * - root directive: /images/cat.jpg -> <root>/images/cat.jpg
     * - alias directive: /photos/cat.jpg -> <alias>/cat.jpg (drops /photos)
     * 
     * PARAMETERS:
     * - uri: Request URI (e.g., "/images/cat.jpg")
     * - location_path: Location block path (e.g., "/images")
     * - config: Configuration with root/alias settings
     * 
     * RETURNS:
     * - Absolute filesystem path
     */
    std::string resolvePath(const std::string& uri,
                           const std::string& location_path,
                           const CommonConfigServerLocation* config) const;
    
    /**
     * isCGIRequest() - Determines if request should be handled by CGI
     * 
     * Checks:
     * - cgi_pass is configured
     * - File extension matches cgi_extensions list
     * 
     * RETURNS:
     * - true if this should be handled as CGI
     * - false for static file serving
     */
    bool isCGIRequest(const std::string& uri,
                     const CommonConfigServerLocation* config) const;
    
    /**
     * readFile() - Reads entire file into string
     * 
     * RETURNS:
     * - File contents, or empty string if file cannot be read
     */
    std::string readFile(const std::string& path) const;
    
    /**
     * fileExists() - Checks if file exists and is readable
     */
    bool fileExists(const std::string& path) const;
    
    /**
     * isDirectory() - Checks if path is a directory
     */
    bool isDirectory(const std::string& path) const;
    
    /**
     * findIndexFile() - Searches for index files in directory
     * 
     * Checks each file in config.index list (e.g., index.html, index.htm).
     * 
     * RETURNS:
     * - Path to first existing index file, or empty string if none found
     */
    std::string findIndexFile(const std::string& directory_path,
                             const CommonConfigServerLocation* config) const;
};

#endif
