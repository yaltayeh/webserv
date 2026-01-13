#include "../include/RequestRouter.hpp"
#include "../dependencies/libhttp/include/Utils.hpp"
#include "../dependencies/libhttp/include/CGI.hpp"
#include "../dependencies/libutils/include/StringUtils.hpp"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>

RequestRouter::RequestRouter() {}

RequestRouter::~RequestRouter() {}

/**
 * route() - Main routing logic
 * 
 * This is the core function that processes an HTTP request and generates
 * an appropriate response based on server configuration.
 * 
    * This is the main entry point for request processing. It:
    * 1. Matches the request URI against location blocks
    * 2. Validates the request method is allowed
    * 3. Checks access control (deny directive)
    * 4. Handles redirects (return directive)
    * 5. Serves static files or executes CGI scripts
    * 6. Generates error pages for failures
 */
libhttp::Response RequestRouter::route(const libhttp::Request& request,
                                       const ServerConfig& config)
{
    std::string uri = request.getUri();
    
    // STEP 1: Find matching location block
    const LocationConfig* location = findMatchingLocation(uri, config);
    
    // STEP 2: Get effective configuration (merged server + location)
    const CommonConfigServerLocation* effective_config = 
        getEffectiveConfig(config, location);
    
    // STEP 3: Check access control
    if (isAccessDenied(effective_config)) {
        return generateErrorResponse(403, effective_config);
    }
    
    // STEP 4: Validate HTTP method
    if (!isMethodAllowed(request.getMethod(), effective_config)) {
        return generateErrorResponse(405, effective_config);
    }
    
    // STEP 5: Check request body size
    if (exceedsMaxBodySize(request, effective_config)) {
        return generateErrorResponse(413, effective_config);
    }
    
    // STEP 6: Handle redirects (return directive)
    if (effective_config->is_return) {
        return handleRedirect(effective_config->return_code, 
                            effective_config->return_url);
    }
    
    // STEP 7: Determine if CGI or static file
    if (isCGIRequest(uri, effective_config)) {
        return handleCGI(request, uri, effective_config);
    }
    
    // STEP 8: Serve static file
    return handleStaticFile(uri, effective_config);
}

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
const LocationConfig* RequestRouter::findMatchingLocation(const std::string& uri,
                                                          const ServerConfig& config) const
{
    const LocationConfig* best_match = NULL;
    size_t longest_match = 0;
    
    // Iterate through all location blocks
    for (size_t i = 0; i < config.locations.size(); ++i) {
        const LocationConfig* loc = config.locations[i];
        const std::string& loc_path = loc->path;
        
        // Check if URI starts with location path
        if (uri.compare(0, loc_path.length(), loc_path) == 0) {
            // This location matches, check if it's more specific
            if (loc_path.length() > longest_match) {
                longest_match = loc_path.length();
                best_match = loc;
            }
        }
    }
    
    return best_match;
}

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
const CommonConfigServerLocation* RequestRouter::getEffectiveConfig(
    const ServerConfig& server_config,
    const LocationConfig* location) const
{
    if (location != NULL) {
        // Location config inherits and overrides server config
        return location;
    }
    
    // No location match, use server config directly
    return &server_config;
}

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
libhttp::Response RequestRouter::handleStaticFile(const std::string& uri,
                                                  const CommonConfigServerLocation* config)
{
    libhttp::Response response;
    
    // Get location path (empty string if server-level config)
    std::string location_path = "";
    const LocationConfig* loc = dynamic_cast<const LocationConfig*>(config);
    if (loc != NULL) {
        location_path = loc->path;
    }
    
    // Resolve URI to filesystem path
    std::string file_path = resolvePath(uri, location_path, config);
    
    std::cout << "Resolving URI '" << uri << "' to path: " << file_path << std::endl;
    
    // Check if path exists
    if (!fileExists(file_path)) {
        return generateErrorResponse(404, config);
    }
    
    // Handle directory
    if (isDirectory(file_path)) {
        // Look for index file
        std::string index_file = findIndexFile(file_path, config);
        
        if (!index_file.empty()) {
            file_path = index_file;
        } else if (config->autoindex) {
            // Generate directory listing
            std::cout << "Autoindex enabled, generating directory listing" << std::endl;
            return handleAutoindex(file_path, uri);
        } else {
            // No index file and autoindex disabled
            std::cout << "Autoindex disabled (autoindex=" << config->autoindex << ")" << std::endl;
            return generateErrorResponse(403, config);
        }
    }
    
    // Read file contents
    std::string content = readFile(file_path);
    if (content.empty() && !fileExists(file_path)) {
        return generateErrorResponse(404, config);
    }
    
    // Build response
    response.setStatus(200);
    response.setHeader("Content-Type", libhttp::getMimeType(file_path));
    
    std::ostringstream ss;
    ss << content.size();
    response.setHeader("Content-Length", ss.str());
    
    response.setBody(content);
    
    return response;
}

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
libhttp::Response RequestRouter::handleCGI(const libhttp::Request& request,
                                           const std::string& uri,
                                           const CommonConfigServerLocation* config)
{
    libhttp::Response response;
    
    // Get location path
    std::string location_path = "";
    const LocationConfig* loc = dynamic_cast<const LocationConfig*>(config);
    if (loc != NULL) {
        location_path = loc->path;
    }
    
    // Resolve script path
    std::string script_path = resolvePath(uri, location_path, config);
    
    std::cout << "CGI: Executing script at " << script_path << std::endl;
    
    // Check script exists
    if (!fileExists(script_path)) {
        return generateErrorResponse(404, config);
    }
    
    // Setup CGI execution
    CGI cgi;
    std::string interpreter = config->cgi_pass;
    std::string server_name = "localhost"; // TODO: Get from server config
    int server_port = 8080; // TODO: Get from server config
    
    CGI::SetupResult setup_result = cgi.setup(request, script_path, interpreter,
                                             server_name, server_port);
    
    if (setup_result != CGI::SETUP_OK) {
        std::cerr << "CGI setup failed: " 
                  << CGI::getSetupResultMessage(setup_result) << std::endl;
        return generateErrorResponse(500, config);
    }
    
    // Execute CGI script
    int read_pipe_fd = -1;
    int write_pipe_fd = -1;
    
    pid_t pid = cgi.execute(read_pipe_fd, write_pipe_fd);
    
    if (pid == -1) {
        std::cerr << "CGI execution failed: " << cgi.getError() << std::endl;
        return generateErrorResponse(500, config);
    }
    
    // Write request body to CGI script (for POST requests)
    std::string request_body = request.getBody();
    if (!request_body.empty() && write_pipe_fd != -1) {
        ssize_t written = write(write_pipe_fd, request_body.c_str(), 
                               request_body.size());
        if (written < 0) {
            std::cerr << "Failed to write to CGI script" << std::endl;
        }
    }
    
    // CRITICAL: Close write pipe to signal EOF to CGI script
    if (write_pipe_fd != -1) {
        close(write_pipe_fd);
    }
    
    // Read CGI output
    std::string cgi_output;
    char buffer[4096];
    ssize_t bytes_read;
    
    while ((bytes_read = read(read_pipe_fd, buffer, sizeof(buffer))) > 0) {
        cgi_output.append(buffer, bytes_read);
    }
    
    close(read_pipe_fd);
    
    // Wait for CGI script to finish
    int status;
    waitpid(pid, &status, 0);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::cerr << "CGI script exited with error" << std::endl;
        return generateErrorResponse(500, config);
    }
    
    // Parse CGI output
    std::map<std::string, std::string> cgi_headers;
    std::string cgi_body;
    
    if (!CGI::parseCGIOutput(cgi_output, cgi_headers, cgi_body)) {
        std::cerr << "Failed to parse CGI output" << std::endl;
        return generateErrorResponse(500, config);
    }
    
    // Build response from CGI output
    response.setStatus(200);
    
    // Copy CGI headers to response
    for (std::map<std::string, std::string>::const_iterator it = cgi_headers.begin();
         it != cgi_headers.end(); ++it) {
        response.setHeader(it->first, it->second);
    }
    
    // Set Content-Length if not already set by CGI
    if (cgi_headers.find("Content-Length") == cgi_headers.end()) {
        std::ostringstream ss;
        ss << cgi_body.size();
        response.setHeader("Content-Length", ss.str());
    }
    
    response.setBody(cgi_body);
    
    return response;
}

/**
 * handleRedirect() - Generates HTTP redirect response
 * 
 * Handles return directive:
 * - return 301 /new-location;
 * - return 302 https://example.com;
 */
libhttp::Response RequestRouter::handleRedirect(int status_code, 
                                                const std::string& url)
{
    libhttp::Response response;
    
    response.setStatus(status_code);
    response.setHeader("Location", url);
    response.setHeader("Content-Length", "0");
    
    return response;
}

/**
 * handleAutoindex() - Generates directory listing
 * 
 * Lists files and subdirectories in HTML format when:
 * - No index file found
 * - autoindex is enabled
 */
libhttp::Response RequestRouter::handleAutoindex(const std::string& directory_path,
                                                 const std::string& uri)
{
    libhttp::Response response;
    
    // Open directory
    DIR* dir = opendir(directory_path.c_str());
    if (dir == NULL) {
        response.setStatus(500);
        response.setBody("Failed to open directory");
        return response;
    }
    
    // Build HTML directory listing
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <title>Index of " << uri << "</title>\n"
         << "  <style>\n"
         << "    body { font-family: monospace; }\n"
         << "    table { border-collapse: collapse; }\n"
         << "    th, td { text-align: left; padding: 8px; }\n"
         << "    tr:hover { background-color: #f5f5f5; }\n"
         << "  </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>Index of " << uri << "</h1>\n"
         << "  <table>\n"
         << "    <tr><th>Name</th><th>Size</th></tr>\n";
    
    // Add parent directory link if not root
    if (uri != "/") {
        html << "    <tr><td><a href=\"../\">../</a></td><td>-</td></tr>\n";
    }
    
    // Read directory entries
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        
        // Skip . and ..
        if (name == "." || name == "..") {
            continue;
        }
        
        // Get file info
        std::string full_path = directory_path + "/" + name;
        struct stat st;
        
        if (stat(full_path.c_str(), &st) == 0) {
            std::string display_name = name;
            std::string size_str;
            
            if (S_ISDIR(st.st_mode)) {
                display_name += "/";
                size_str = "-";
            } else {
                std::ostringstream ss;
                ss << st.st_size;
                size_str = ss.str();
            }
            
            // Ensure URI ends with / for proper relative links
            std::string base_uri = uri;
            if (!base_uri.empty() && base_uri[base_uri.length() - 1] != '/') {
                base_uri += "/";
            }
            
            html << "    <tr>"
                 << "<td><a href=\"" << base_uri << name << "\">" 
                 << display_name << "</a></td>"
                 << "<td>" << size_str << "</td>"
                 << "</tr>\n";
        }
    }
    
    closedir(dir);
    
    html << "  </table>\n"
         << "</body>\n"
         << "</html>\n";
    
    std::string body = html.str();
    
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    
    std::ostringstream ss;
    ss << body.size();
    response.setHeader("Content-Length", ss.str());
    
    response.setBody(body);
    
    return response;
}

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
libhttp::Response RequestRouter::generateErrorResponse(int status_code,
                                                       const CommonConfigServerLocation* config)
{
    // Check for custom error page
    std::map<int, CommonConfig::ErrorPage>::const_iterator it = 
        config->error_pages.find(status_code);
    
    if (it != config->error_pages.end()) {
        const CommonConfig::ErrorPage& error_page = it->second;
        std::string error_file_path = config->root + error_page.uri;
        
        // Try to serve custom error page
        if (fileExists(error_file_path)) {
            std::string content = readFile(error_file_path);
            
            libhttp::Response response;
            response.setStatus(status_code);
            response.setHeader("Content-Type", "text/html");
            
            std::ostringstream ss;
            ss << content.size();
            response.setHeader("Content-Length", ss.str());
            
            response.setBody(content);
            return response;
        }
    }
    
    // Fall back to default error page
    return generateDefaultErrorPage(status_code);
}

/**
 * generateDefaultErrorPage() - Creates default HTML error page
 * 
 * Used when no custom error_page is configured.
 */
libhttp::Response RequestRouter::generateDefaultErrorPage(int status_code)
{
    libhttp::Response response;
    
    std::string message = libhttp::getStatusMessage(status_code);
    
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <title>" << status_code << " " << message << "</title>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>" << status_code << " " << message << "</h1>\n"
         << "  <hr>\n"
         << "  <p>webserv/1.0</p>\n"
         << "</body>\n"
         << "</html>\n";
    
    std::string body = html.str();
    
    response.setStatus(status_code);
    response.setHeader("Content-Type", "text/html");
    
    std::ostringstream ss;
    ss << body.size();
    response.setHeader("Content-Length", ss.str());
    
    response.setBody(body);
    
    return response;
}

/**
 * isMethodAllowed() - Checks if HTTP method is allowed for this location
 * 
 * RETURNS:
 * - true if method is allowed (or no restrictions configured)
 * - false if method is explicitly denied
 */
bool RequestRouter::isMethodAllowed(libhttp::Method method,
                                   const CommonConfigServerLocation* config) const
{
    // If no restrictions configured, all methods are allowed
    if (config->allowed_methods.empty()) {
        return true;
    }
    
    std::string method_str = libhttp::methodToString(method);
    
    return config->allowed_methods.find(method_str) != config->allowed_methods.end();
}

/**
 * isAccessDenied() - Checks if access is denied (deny directive)
 * 
 * RETURNS:
 * - true if deny_all is enabled
 * - false otherwise
 */
bool RequestRouter::isAccessDenied(const CommonConfigServerLocation* config) const
{
    return config->deny_all;
}

/**
 * exceedsMaxBodySize() - Checks if request body exceeds limit
 * 
 * RETURNS:
 * - true if Content-Length exceeds client_max_body_size
 * - false otherwise
 */
bool RequestRouter::exceedsMaxBodySize(const libhttp::Request& request,
                                      const CommonConfigServerLocation* config) const
{
    std::string content_length_str = request.getHeader("Content-Length");
    
    if (content_length_str.empty()) {
        return false; // No Content-Length header, cannot check
    }
    
    // Parse Content-Length
    size_t content_length = std::atol(content_length_str.c_str());
    
    return content_length > config->client_max_body_size;
}

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
std::string RequestRouter::resolvePath(const std::string& uri,
                                      const std::string& location_path,
                                      const CommonConfigServerLocation* config) const
{
    const LocationConfig* loc = dynamic_cast<const LocationConfig*>(config);
    
    // Check for alias directive
    if (loc != NULL && !loc->alias.empty()) {
        // alias: strip location path from URI and prepend alias
        // Example: location /photos { alias /data/images; }
        // URI /photos/cat.jpg -> /data/images/cat.jpg
        
        std::string remaining_path = uri.substr(location_path.length());
        return loc->alias + remaining_path;
    }
    
    // Use root directive (default behavior)
    // Example: root /var/www;
    // URI /images/cat.jpg -> /var/www/images/cat.jpg
    return config->root + uri;
}

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
bool RequestRouter::isCGIRequest(const std::string& uri,
                                const CommonConfigServerLocation* config) const
{
    // Check if CGI is configured
    if (config->cgi_pass.empty()) {
        return false;
    }
    
    // Check if URI has CGI extension
    for (size_t i = 0; i < config->cgi_extensions.size(); ++i) {
        const std::string& ext = config->cgi_extensions[i];
        
        // Check if URI ends with this extension
        if (uri.length() >= ext.length()) {
            if (uri.compare(uri.length() - ext.length(), ext.length(), ext) == 0) {
                return true;
            }
        }
    }
    
    return false;
}

/**
 * readFile() - Reads entire file into string
 * 
 * RETURNS:
 * - File contents, or empty string if file cannot be read
 */
std::string RequestRouter::readFile(const std::string& path) const
{
    std::ifstream file(path.c_str(), std::ios::binary);
    
    if (!file.is_open()) {
        return "";
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read file into string
    std::string content;
    content.resize(size);
    
    if (!file.read(&content[0], size)) {
        return "";
    }
    
    return content;
}

/**
 * fileExists() - Checks if file exists and is readable
 */
bool RequestRouter::fileExists(const std::string& path) const
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

/**
 * isDirectory() - Checks if path is a directory
 */
bool RequestRouter::isDirectory(const std::string& path) const
{
    struct stat st;
    
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    
    return S_ISDIR(st.st_mode);
}

/**
 * findIndexFile() - Searches for index files in directory
 * 
 * Checks each file in config.index list (e.g., index.html, index.htm).
 * 
 * RETURNS:
 * - Path to first existing index file, or empty string if none found
 */
std::string RequestRouter::findIndexFile(const std::string& directory_path,
                                        const CommonConfigServerLocation* config) const
{
    // Ensure directory path ends with /
    std::string dir = directory_path;
    if (!dir.empty() && dir[dir.length() - 1] != '/') {
        dir += "/";
    }
    
    // Check each index file in order
    for (size_t i = 0; i < config->index.size(); ++i) {
        std::string index_path = dir + config->index[i];
        
        if (fileExists(index_path) && !isDirectory(index_path)) {
            return index_path;
        }
    }
    
    return ""; // No index file found
}
