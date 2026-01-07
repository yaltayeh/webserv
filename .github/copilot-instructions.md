# Webserv - HTTP/1.0 Server (42 School Project)

## Project Overview
This is a C++98 HTTP/1.0 web server implementation following 42 School requirements. The server must be non-blocking, event-driven (using `poll()`/`select()`/`kqueue()`), and handle multiple simultaneous connections without threads or external libraries.

## Architecture

### Two-Layer Design
- **Main Server (`src/main.cpp`)**: Event loop, socket management, I/O multiplexing, configuration parsing, CGI execution
- **HTTP Library (`dependencies/libhttp/`)**: HTTP protocol parsing and response generation (modular, reusable)

### libhttp Library Structure
- `Parser`: Stateful incremental HTTP request parser handling partial reads (REQUEST_LINE → HEADERS → BODY → COMPLETE)
- `Request`: HTTP request representation (method, URI, headers, body)
- `Response`: HTTP response builder (status code, headers, body)
- `Constants`: HTTP methods (GET/POST/DELETE), status codes, MIME types
- `Utils`: String manipulation, MIME type detection, method conversions

**Key Pattern**: `Parser` accumulates data across multiple `parse()` calls, returning 0 (complete), 1 (incomplete), or -1 (error). This enables non-blocking incremental parsing.

## Build System

### Nested Makefile Pattern
```bash
make              # Builds dependencies first, then main executable
make re           # Full rebuild (fclean + all)
```

- Main Makefile builds `build/webserv` and orchestrates dependency builds
- Dependency makefiles use `BUILD_PATH=../../build/libname` for consistent output structure
- Compiled objects: `build/obj/*.o` and `build/libhttp/obj/*.o`
- Static library: `build/libhttp/libhttp.a`
- Final executable: `build/webserv`

**Critical**: Never hardcode build paths. Use `BUILD_PATH ?= ./build` pattern for flexibility.

## Strict Requirements (42 School)

### Mandatory Constraints
- **C++98 only**: No C++11/14/17 features (`-std=c++98` enforced)
- **No external libraries**: No Boost, no third-party deps except standard library
- **Single I/O multiplexer**: One `poll()`/`select()`/`kqueue()` call for all I/O operations
- **Non-blocking I/O**: All sockets/pipes must use `fcntl(F_SETFL, O_NONBLOCK)`. Never read/write without readiness check
- **No errno checking**: Cannot adjust behavior based on `errno` after read/write (forces proper readiness handling)
- **Process model**: `fork()` allowed ONLY for CGI execution

### Allowed System Calls
See `subject.md` for complete list. Key functions: `socket`, `bind`, `listen`, `accept`, `poll`/`select`/`kqueue`, `fcntl`, `fork` (CGI only), `waitpid`, `pipe`, `dup2`, `execve` (CGI)

## Configuration File Requirements

Must support NGINX-like syntax for:
- Multiple `server` blocks (listen on different host:port pairs)
- Route-specific rules (accepted methods, redirects, root directory, directory listing, default files)
- Client body size limits and error pages
- CGI execution based on file extensions (e.g., `.php` → `php-cgi`)
- File upload locations

**CGI Specifics**: 
- Must un-chunk chunked requests before passing to CGI
- Set proper environment variables (REQUEST_METHOD, QUERY_STRING, CONTENT_LENGTH, etc.)
- Execute CGI in correct directory for relative paths
- Handle both Content-Length and EOF-based CGI output

## Code Conventions

### C++ Style
- **Headers**: Include guards (e.g., `#ifndef REQUEST_HPP`)
- **Namespaces**: All libhttp code in `libhttp::` namespace
- **Naming**: PascalCase for classes/enums, camelCase for methods, `_underscore` prefix for private members
- **Operators**: Implement `operator<<` for stream insertion on Request/Response classes
- **Memory**: Orthodox Canonical Form (copy constructor, assignment operator, destructor) for all classes

### Error Handling Pattern
```cpp
if (condition_failed) {
    _state = ERROR;
    return -1;
}
```
Return -1 for errors, 0 for success, 1 for incomplete (in Parser).

## Development Workflow

### Testing Strategy
- Use `src/example_of_libhttp.cpp` to test libhttp in isolation (incremental parsing example provided)
- Test with real browsers and `curl`/`telnet` for protocol compliance
- Compare behavior with NGINX when uncertain about HTTP semantics
- Must pass stress tests (server remains available under load)

### Debugging Non-Blocking I/O
- Verify `fcntl(fd, F_SETFL, O_NONBLOCK)` on all sockets
- Check `poll()`/`select()` return value and `revents` before read/write
- MacOS: `write()` behaves differently; use `fcntl()` with `F_SETFL, O_NONBLOCK, FD_CLOEXEC` only

### CGI Testing
- Test PHP-CGI or Python scripts with query strings, POST data, and environment variables
- Verify chunked transfer encoding is properly decoded before forwarding to CGI
- Check working directory is correct for CGI script execution

## Key Files to Modify
- `src/main.cpp`: Main event loop, socket setup, request routing, CGI execution
- `dependencies/libhttp/`: HTTP protocol logic (already implements parsing foundation)
- Configuration file parser (not yet implemented - must create)
- Error page handlers (must implement default pages)

## Reference Materials
- HTTP/1.0 RFC 1945 (recommended reading)
- NGINX configuration syntax (inspiration for config file)
- `subject.md`: Complete project requirements
- `src/example_of_libhttp.cpp`: Example of incremental parsing pattern
