# All Available Directives - Quick Reference

## Block Directives
1. **http** - Top-level HTTP configuration block (context: main)
2. **server** - Virtual server block (context: http)
3. **location** [modifier] uri - Location-specific rules (context: server, location)

## Simple Directives

### Server Identity
4. **server_name** name [name2...] - Server names for virtual hosting (context: server)
5. **listen** address:port - Listening address and port (context: server)

### Paths & Files
6. **root** path - Document root directory (context: http, server, location)
7. **index** file [file2...] - Default index files (context: http, server, location)
8. **autoindex** on|off - Directory listing (context: http, server, location)

### HTTP Methods & Responses
10. **allowed_methods** METHOD [METHOD2...] - Restrict HTTP methods (context: location)
11. **limit_except** METHOD [METHOD2...] { ... } - Limit allowed methods (context: location)
12. **return** code [text] - Return HTTP response (context: server, location)
13. **error_page** code [code2...] uri - Custom error pages (context: http, server, location)

### Access Control
21. **deny** address - Deny access from address/CIDR/all (context: http, server, location)

### Request Limits
13. **client_body_temp_path** path [level1 [level2 [level3]]] - Temp directory for request bodies (context: http, server, location)
### Request Limits
13. **client_body_temp_path** path [level1 [level2 [level3]]] - Temp directory for request bodies (context: http, server, location)
14. **client_max_body_size** size - Max request body size (context: http, server, location)
15. **keepalive_timeout** timeout [header_timeout] - Keepalive timeout (context: http, server, location)
16. **proxy_request_buffering** on|off - Buffer client request body for proxying (context: http, server, location)

### File Uploads
17. **upload_enable** on|off - Enable file uploads (context: location)
18. **upload_store** path - Upload storage directory (context: location)

### CGI Execution
19. **cgi_pass** interpreter_path - CGI interpreter path (context: location)
20. **cgi_param** name value - CGI environment variable (context: location)
21. **cgi_extension** ext [ext2...] - CGI file extensions (context: http, server, location)

## Total: 23 Directives

## Implementation Files

All directives are in:
- Headers: `directives/include/*.hpp`
- Sources: `directives/src/*.cpp`

Built with:
```bash
make re
```

## Context Hierarchy

```
main (GlobalDirective)
└── http
    ├── server
    │   └── location
    │       └── location (nested)
    └── server
        └── location
```

## Common Patterns

**Multi-value directives:**
- server_name (names)
- index (files)
- allowed_methods (methods)
- error_page (codes)
- cgi_extension (extensions)
- cgi_param (multiple instances)

**Boolean directives:**
- autoindex (on/off)
- upload_enable (on/off)

**Size directives:**
- client_max_body_size (with units: k, m, g)

**Time directives:**
- keepalive_timeout (with units: s, m, h)

**Path directives:**
- root
- upload_store
- cgi_pass

**Code-based directives:**
- return (HTTP status codes)
- error_page (HTTP error codes)

## All Data Types

```cpp
// Strings
const std::string path, host, size, timeout, uri, text, name, value, interpreterPath

// Integers
const int port, code

// Booleans
const bool enabled

// String vectors
const std::vector<std::string> names, methods, files, extensions

// Integer vectors
const std::vector<int> codes
```
