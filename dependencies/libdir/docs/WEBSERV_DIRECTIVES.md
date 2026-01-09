# Webserv Project Directives

This document describes all the directives implemented for the webserv project requirements.

## Block Directives

### http
- **Syntax:** `http { ... }`
- **Context:** main
- **Description:** Top-level block containing all server configurations
- **Data:** No data members (block directive only)

### server
- **Syntax:** `server { ... }`
- **Context:** http
- **Description:** Defines a virtual server configuration
- **Data:** No data members (block directive only)

### location
- **Syntax:** `location [modifier] uri { ... }`
- **Context:** server, location
- **Description:** Defines location-specific configuration rules
- **Modifiers:**
  - `=` - Exact match
  - `@` - Named location (internal redirects)
  - (none) - Prefix match
- **Data:**
  - `const std::string modifier` - Match modifier type
  - `const std::string uri` - URI pattern
- **Nesting:** Supports nested location blocks

## Server Configuration

### server_name
- **Syntax:** `server_name name1 [name2 ...];`
- **Context:** server
- **Description:** Sets server names for virtual hosting
- **Data:** `const std::vector<std::string> names`
- **Example:** `server_name localhost example.com www.example.com;`

### listen
- **Syntax:** `listen address:port;` or `listen port [host];`
- **Context:** server
- **Description:** Specifies server listening address and port
- **Data:**
  - `const int port` - Port number
  - `const std::string host` - IP address or hostname
- **Example:** 
  - `listen 127.0.0.1:8080;`
  - `listen 8080 0.0.0.0;`

## Path and File Configuration

### root
- **Syntax:** `root path;`
- **Context:** http, server, location
- **Description:** Sets the root directory for requests
- **Data:** `const std::string path`
- **Example:** `root /var/www/html;`

### alias
- **Syntax:** `alias path;`
- **Context:** location
- **Description:** Defines a replacement for the specified location (unlike root which appends)
- **Data:** `const std::string path`
- **Example:** 
  ```nginx
  location /images/ {
      alias /data/pictures/;
  }
  # Request /images/photo.jpg serves /data/pictures/photo.jpg
  ```
- **Note:** The location path is replaced entirely, not appended like with root
- **Difference from root:**
  - `root /data;` with `location /images/` serves `/data/images/photo.jpg`
  - `alias /data/pictures/;` with `location /images/` serves `/data/pictures/photo.jpg`

### index
- **Syntax:** `index file1 [file2 ...];`
- **Context:** http, server, location
- **Description:** Defines default index files
- **Data:** `const std::vector<std::string> files`
- **Example:** `index index.html index.htm default.html;`

### autoindex
- **Syntax:** `autoindex on|off;`
- **Context:** http, server, location
- **Description:** Enables or disables directory listing
- **Data:** `const bool enabled`
- **Example:** `autoindex on;`

## HTTP Configuration

### allowed_methods
- **Syntax:** `allowed_methods METHOD1 [METHOD2 ...];`
- **Context:** location
- **Description:** Restricts which HTTP methods are allowed
- **Data:** `const std::vector<std::string> methods`
- **Valid Methods:** GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH, CONNECT, TRACE
- **Example:** `allowed_methods GET POST;`

### return
- **Syntax:** `return code [text];`
- **Context:** server, location
- **Description:** Returns HTTP response with specified status code
- **Data:**
  - `const int code` - HTTP status code (100-599)
  - `const std::string text` - Optional redirect URL or response text
- **Examples:**
  - `return 301 https://new-site.com;` (redirect)
  - `return 403;` (forbidden)
  - `return 444;` (close connection)

### error_page
- **Syntax:** `error_page code1 [code2 ...] uri;`
- **Context:** http, server, location
- **Description:** Defines custom error pages for HTTP status codes
- **Data:**
  - `const std::vector<int> codes` - Error codes
  - `const std::string uri` - Error page URI
- **Example:** `error_page 404 /404.html;`

## Access Control

### deny
- **Syntax:** `deny address;`
- **Context:** http, server, location
- **Description:** Denies access from specified address
- **Data:** `const std::string address`
- **Valid Values:**
  - IP address: `192.168.1.1`
  - CIDR notation: `192.168.1.0/24`
  - `all` - deny all addresses
- **Example:** 
  - `deny 192.168.1.1;`
  - `deny 10.0.0.0/8;`
  - `deny all;`

### limit_except
- **Syntax:** `limit_except method [method2 ...] { ... }`
- **Context:** location
- **Description:** Limits allowed HTTP methods (block directive)
- **Data:** `const std::vector<std::string> methods`
- **Valid Methods:** GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH, CONNECT, TRACE
- **Behavior:** Listed methods are ALLOWED, all others are DENIED
- **Example:**
```nginx
location /api/ {
    limit_except GET POST {
        deny all;  # Only GET and POST allowed, deny all for other methods
    }
}
```
- **Note:** Can contain other directives like `deny` to further restrict access

## Request Limits

### client_body_temp_path
- **Syntax:** `client_body_temp_path path [level1 [level2 [level3]]];`
- **Context:** http, server, location
- **Description:** Defines a directory for storing temporary files with client request bodies
- **Data:** `const std::string path`, `const int level1`, `const int level2`, `const int level3`
- **Valid Values:** 
  - `path` - directory path
  - `level1`, `level2`, `level3` - subdirectory levels (0-10, optional)
- **Example:** 
  - `client_body_temp_path /var/tmp/client_body;`
  - `client_body_temp_path /var/tmp/client_body 1 2;`
- **Note:** Subdirectory levels define the hierarchy of subdirectories for temporary files

### client_max_body_size
- **Syntax:** `client_max_body_size size;`
- **Context:** http, server, location
- **Description:** Sets maximum allowed size of client request body
- **Data:** `const std::string size`
- **Format:** Number with optional suffix (k, m, g)
- **Example:** `client_max_body_size 10M;`

### keepalive_timeout
- **Syntax:** `keepalive_timeout timeout [header_timeout];`
- **Context:** http, server, location
- **Description:** Sets keepalive connection timeout
- **Data:**
  - `const std::string timeout` - Server timeout
  - `const std::string headerTimeout` - Optional header timeout
- **Example:** `keepalive_timeout 60s 30s;`

### proxy_request_buffering
- **Syntax:** `proxy_request_buffering on|off;`
- **Context:** http, server, location
- **Description:** Enables or disables buffering of client request body for proxying
- **Data:** `const bool enabled`
- **Valid Values:** `on` (enabled), `off` (disabled)
- **Example:** `proxy_request_buffering off;`
- **Note:** When disabled, request body is sent to proxied server immediately without buffering

## File Upload

### upload_enable
- **Syntax:** `upload_enable on|off;`
- **Context:** location
- **Description:** Enables or disables file uploads
- **Data:** `const bool enabled`
- **Example:** `upload_enable on;`

### upload_store
- **Syntax:** `upload_store path;`
- **Context:** location
- **Description:** Sets directory for uploaded files
- **Data:** `const std::string path`
- **Example:** `upload_store /var/www/uploads;`

## CGI Configuration

### cgi_pass
- **Syntax:** `cgi_pass interpreter_path;`
- **Context:** location
- **Description:** Sets path to CGI interpreter executable
- **Data:** `const std::string interpreterPath`
- **Example:** 
  - `cgi_pass /usr/bin/php-cgi;`
  - `cgi_pass /usr/bin/python3;`

### cgi_param
- **Syntax:** `cgi_param name value;`
- **Context:** location
- **Description:** Sets CGI environment variable
- **Data:**
  - `const std::string name` - Parameter name
  - `const std::string value` - Parameter value
- **Example:**
  - `cgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;`
  - `cgi_param QUERY_STRING $query_string;`
- **Note:** Can be used multiple times for multiple parameters

### cgi_extension
- **Syntax:** `cgi_extension ext1 [ext2 ...];`
- **Context:** http, server, location
- **Description:** Defines file extensions that should be processed by CGI
- **Data:** `const std::vector<std::string> extensions`
- **Example:** `cgi_extension .php .py .pl;`
- **Note:** Automatically adds '.' prefix if missing (e.g., "php" → ".php")

## Complete Example

```nginx
http {
    # HTTP-level defaults
    client_max_body_size 10M;
    keepalive_timeout 60s;
    error_page 500 502 503 /50x.html;
    cgi_extension .php .py;
    
    server {
        listen 127.0.0.1:8080;
        server_name localhost example.com;
        root /var/www/html;
        index index.html index.htm;
        autoindex off;
        
        # Main location
        location / {
            allowed_methods GET POST;
        }
        
        # API redirect
        location /api {
            return 301 https://api.example.com;
        }
        
        # File downloads with directory listing
        location /downloads {
            allowed_methods GET;
            root /var/www/downloads;
            autoindex on;
        }
        
        # File uploads
        location /upload {
            allowed_methods POST PUT DELETE;
            upload_enable on;
            upload_store /var/www/uploads;
            client_max_body_size 50M;
        }
        
        # PHP CGI
        location ~ \.php$ {
            allowed_methods GET POST;
            root /var/www/cgi-bin;
            cgi_pass /usr/bin/php-cgi;
            cgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
            cgi_param QUERY_STRING $query_string;
        }
    }
}
```

## Access in C++

All directives can be accessed via typed getters:

```cpp
// Get server configuration
const Config::ServerDirective* server = http->getFirstChild<Config::ServerDirective>();

// Get listen directive
const Config::ListenDirective* listen = server->getFirstChild<Config::ListenDirective>();
std::cout << "Port: " << listen->port << std::endl;

// Get all locations
std::vector<Config::LocationDirective*> locations = server->getChildren<Config::LocationDirective>();

// Get allowed methods
const Config::AllowedMethodsDirective* methods = 
    location->getFirstChild<Config::AllowedMethodsDirective>();
for (size_t i = 0; i < methods->methods.size(); ++i) {
    std::cout << methods->methods[i] << std::endl;
}

// Get CGI parameters (multiple instances)
std::vector<Config::CgiParamDirective*> params = 
    location->getChildren<Config::CgiParamDirective>();
for (size_t i = 0; i < params.size(); ++i) {
    std::cout << params[i]->name << " = " << params[i]->value << std::endl;
}
```

## Optional vs Required

All directives are optional at the parser level. Wrap `getFirstChild` calls in try-catch to handle missing directives:

```cpp
try {
    const Config::RootDirective* root = server->getFirstChild<Config::RootDirective>();
    std::string path = root->path;
} catch (const Config::DirectiveNotFound&) {
    // Use default or inherit from parent
}
```

## Testing

See `tests/test_webserv.cpp` for a comprehensive example demonstrating all directives in action.

Run the test:
```bash
make re
c++ -Wall -Wextra -Werror -std=c++98 -I./include -I./directives/include \
    tests/test_webserv.cpp build/libdir.a -o test_webserv
./test_webserv
```
