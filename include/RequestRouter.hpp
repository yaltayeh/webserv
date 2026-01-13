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
    
    libhttp::Response route(const libhttp::Request& request, const ServerConfig& config);

private:
    // Disable copy
    RequestRouter(const RequestRouter& other);
    RequestRouter& operator=(const RequestRouter& other);
    
    
    const LocationConfig findMatchingLocation(const std::string& uri,
                                               const ServerConfig& config) const;
    
    
    libhttp::Response handleStaticFile(const std::string& uri,
                                       const CommonConfigServerLocation& config);
    
    libhttp::Response handleCGI(const libhttp::Request& request,
                                const std::string& uri,
                                const CommonConfigServerLocation& config);
    
    libhttp::Response handleRedirect(int status_code, const std::string& url);
    
    libhttp::Response handleAutoindex(const std::string& directory_path,
                                      const std::string& uri);
    
    // ===== ERROR HANDLING =====
    
    libhttp::Response generateErrorResponse(int status_code,
                                            const CommonConfigServerLocation& config);
    
    libhttp::Response generateDefaultErrorPage(int status_code);
    
    // ===== VALIDATION =====
    
    bool isMethodAllowed(libhttp::Method method,
                        const CommonConfigServerLocation& config) const;
    
    bool isAccessDenied(const CommonConfigServerLocation& config) const;
    
    bool exceedsMaxBodySize(const libhttp::Request& request,
                           const CommonConfigServerLocation& config) const;
    
    // ===== UTILITIES =====
    
    std::string resolvePath(const std::string& uri,
                           const std::string& location_path,
                           const CommonConfigServerLocation& config) const;
    
    bool isCGIRequest(const std::string& uri,
                     const CommonConfigServerLocation& config) const;
    
    std::string readFile(const std::string& path) const;

    bool fileExists(const std::string& path) const;
  
    bool isDirectory(const std::string& path) const;
    
    std::string findIndexFile(const std::string& directory_path,
                             const CommonConfigServerLocation& config) const;
};

#endif
