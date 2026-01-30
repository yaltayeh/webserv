# Agent Guidelines for webserv

This document provides coding agents with essential information about the webserv HTTP server project, including build instructions, testing procedures, and code style conventions.

## Project Overview

**webserv** is a C++98 HTTP server implementation following nginx-like configuration syntax. The server uses an event-driven architecture with epoll (Linux) or kqueue (macOS) for handling concurrent connections efficiently.

### Key Architecture Components

- **EventLoop**: Core event loop handling I/O multiplexing (epoll/kqueue)
- **VirtualServer**: Virtual host management for routing requests
- **Socket Hierarchy**: ListenSocket, ClientSocket, StreamSocket for connection handling
- **Middleware Pipeline**: Express.js-style request processing chain in `routing/`
- **Directive System**: Configuration parser using factory pattern in `src/directives/`
- **Dependencies**: Three internal libraries (libhttp, libutils, libdir) in `dependencies/`

---

## Build & Test Commands

### Building the Project

```bash
# Clean build from scratch
make fclean && make

# Build with custom buffer size (default: 8192)
make BUFFER_SIZE=16384

# Build dependencies only
make deps

# Clean build artifacts (keeps binaries)
make clean

# Remove everything including binaries
make fclean

# Rebuild everything
make re
```

### Running the Server

```bash
# Start server with config file
./build/webserv <config-file>

# Example with test config
./build/webserv test.conf
```

### Testing

#### Python Test Framework (Primary)

```bash
# Run all tests using Luka framework
cd luka && python3 runner.py

# Test files located in: luka/tests/
# Each test inherits from BaseTest (luka/base_test.py)
```

Test framework features:
- Auto-compiles the server before running tests
- Starts/stops server for each test automatically
- Tests include: basic requests, CGI, autoindex, error pages, concurrent connections, etc.

#### C++ Unit Tests (Limited)

```bash
# Build and run C++ test suite (minimal usage)
make test
```

#### Running a Single Test

To run a specific test, you need to edit `luka/runner.py` or run tests programmatically. The framework doesn't currently support command-line test filtering, but you can:

1. Navigate to `luka/tests/` and identify test classes
2. Each test class has a `get_name()` method (e.g., "basic_request", "test_cgi")
3. Modify the test runner or run individual test files directly

---

## Code Style Guidelines

### Language & Standards

- **C++ Standard**: C++98 (strict adherence required)
- **Compiler**: c++ (g++ or clang++)
- **Flags**: `-Wall -Wextra -Werror -std=c++98`
- **No STL algorithms**: Prefer manual loops over `<algorithm>` functions
- **No C++11+**: No `auto`, range-based for, `nullptr`, lambda functions, etc.

### File Organization

#### Header Files (.hpp)

```cpp
#pragma once

#include <standard_headers>
#include "project_headers"
#include "libhttp/include/Request.hpp"  // Explicit lib paths

// Use namespaces for modules (routing, libhttp, etc.)
namespace routing {
    class MyClass {
        // Class definition
    };
}
```

#### Source Files (.cpp)

```cpp
#include "MyClass.hpp"
#include <system_headers>
#include "other_project_headers"
#include "libutils/include/Logger.hpp"

// Implementation follows header namespace
namespace routing {
    // Implementation
}
```

### Import/Include Order

1. **Own header first** (for .cpp files): `#include "MyClass.hpp"`
2. **System headers**: `<iostream>`, `<string>`, `<vector>`, etc.
3. **Project headers**: Local includes from `include/`
4. **Library headers**: `libhttp/include/`, `libutils/include/`, `libdir/include/`

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| **Classes/Structs** | PascalCase | `ClientSocket`, `EventLoop` |
| **Functions/Methods** | camelCase | `handleEvent()`, `getSocketfd()` |
| **Member Variables** | _camelCase (underscore prefix) | `_eventFd`, `_running` |
| **Constants** | UPPER_SNAKE_CASE | `READ_BUFFER_SIZE` |
| **Namespaces** | lowercase | `routing`, `libhttp` |
| **Files** | Match class name | `ClientSocket.cpp`, `ClientSocket.hpp` |

### Formatting & Indentation

- **Indentation**: Tabs (not spaces)
- **Braces**: Opening brace on same line for functions, control structures
- **Line Length**: Keep reasonable (<120 chars preferred)
- **Whitespace**: Space after keywords (`if`, `while`), no space after function name

```cpp
// Good
void functionName(int param)
{
	if (condition)
	{
		// code
	}
}

// For single-statement blocks, braces are optional but preferred
if (error)
	return false;
```

### Types & Variables

- **Use explicit types**: No `auto` (C++98)
- **Initialize variables**: Always initialize at declaration when possible
- **Const correctness**: Use `const` for read-only parameters and methods
- **Pointers vs References**: 
  - Use references for non-null parameters
  - Use pointers when NULL is a valid state or for ownership transfer

```cpp
// Good
void processRequest(const Request& request, Response* response);
std::string getValue() const;

// Bad (non-const method that doesn't modify state)
std::string getValue();
```

### Error Handling

#### Exceptions

```cpp
// Use std::runtime_error for exceptional conditions
throw std::runtime_error("Failed to bind socket");

// Build error messages with ostringstream
std::ostringstream oss;
oss << "Error on port " << port << ": " << strerror(errno);
throw std::runtime_error(oss.str());

// Catch in main() or high-level functions
try {
	// code
} catch (const std::exception& e) {
	LOG_ERROR(e.what());
	return 1;
}
```

#### Return Values

- Return `-1` or `false` for error conditions in low-level functions
- Check system call return values immediately
- Use `errno` for system error details

### Logging

Use the Logger macros from `libutils/include/Logger.hpp`:

```cpp
#include "libutils/include/Logger.hpp"

// Log levels (bit mask):
LOG_TRACE("Function entry/exit details");
LOG_DEBUG("Debug info: " << variable);
LOG_INFO("Server started on port " << port);
LOG_WARNING("Client exceeded body size limit");
LOG_ERROR("Failed to open file: " << filename);
LOG_CRITICAL("Fatal error before crash");

// Errno-aware logging:
LOG_ERROR_ERRNO("bind() failed");  // Appends strerror(errno)
LOG_CRITICAL_ERRNO("Memory allocation failed");
```

**Configuration**: Logger level controlled via config file directives:
- `log_level` (TRACE|DEBUG|INFO|WARNING|ERROR|CRITICAL|ALL|NONE)
- `log_colored` (on|off)
- `log_show_timestamp` (on|off)
- `log_show_location` (on|off) - Shows file:line
- `log_file` (path) - Write to file instead of stdout/stderr

### Memory Management

- **Manual memory management**: Use `new`/`delete` (no smart pointers in C++98)
- **Ownership**: Document who owns pointers (caller vs callee)
- **Cleanup**: Always pair `new` with `delete` in destructors
- **Vectors**: Use `std::vector` for dynamic arrays

```cpp
// Document ownership clearly
/**
 * process() - Returns Response* that MUST be deleted by caller
 */
Response* process(const Request& request);

// Cleanup in destructors
~ClientSocket() {
	delete _parser;
	delete _response;
}
```

### Middleware Pattern

The routing system uses a middleware pipeline (see `include/routing/RouteHandler.hpp`):

```cpp
// Middleware signature
typedef Response* (*MiddlewareFunc)(
	const Request& request,
	const LocationConfig& location,
	RouteContext& context
);

// Return NULL to continue to next middleware
// Return Response* to handle request and stop chain
```

### Comments & Documentation

- **Header files**: Use detailed block comments explaining class purpose
- **Complex functions**: Document parameters, return values, and behavior
- **TODO comments**: Use `TODO:` prefix (see main.cpp for examples)

```cpp
/**
 * handleEvent() - Process I/O events for this client
 * 
 * PARAMETERS:
 * - el: Event loop managing this socket
 * 
 * RETURNS:
 * - IO_EVENT_CLOSE: Close connection
 * - IO_EVENT_CONTINUE: Keep connection alive
 */
IOEventState handleEvent(EventLoop* el);
```

---

## Project-Specific Notes

### System Calls & Restrictions

- **Allowed fcntl flags ONLY**: `F_SETFL`, `O_NONBLOCK`, `FD_CLOEXEC`
- All sockets and file descriptors MUST be non-blocking
- Use `FD_CLOEXEC` to prevent fd leaks to child processes

### Configuration System

- Nginx-like syntax parsed by `libdir`
- Directives registered via factory pattern in `src/directives/`
- Each directive inherits from base directive class
- Configuration stored in hierarchical structs: `GlobalConfig` → `HttpConfig` → `ServerConfig` → `LocationConfig`

### Important Files to Check Before Changes

- `Makefile`: Build configuration and dependencies
- `include/BufferConfig.hpp`: Buffer size configuration (compile-time)
- `include/routing/RouteHandler.hpp`: Middleware system architecture
- `dependencies/libutils/include/Logger.hpp`: Logging system API

---

## Common Tasks

### Adding a New Directive

1. Create `include/directives/MyDirective.hpp`
2. Create `src/directives/MyDirective.cpp`
3. Register in `DirectiveFactory` initialization
4. Add to appropriate config struct (`ServerConfig`, `LocationConfig`, etc.)

### Adding a New Middleware

1. Create file in `src/routing/middleware/`
2. Implement function matching `MiddlewareFunc` signature
3. Register in `src/routing/RoutingInit.cpp` with appropriate priority
4. Priority guidelines:
   - 100: Permission checks (body size, access control, method validation)
   - 50: Content handlers (redirects, special routes)
   - 25: Dynamic content (CGI, FastCGI)
   - 0: Default handlers (static files, fallback)

### Debugging

- Enable detailed logging: Set `log_level` to `DEBUG` or `TRACE` in config
- Enable `log_show_location on` to see file:line numbers
- Use `LOG_DEBUG()` liberally during development
- Check `server_log.txt` if `log_file` directive is set

---

## Testing Best Practices

1. **Always run full test suite** before committing: `cd luka && python3 runner.py`
2. **Add tests for new features** in `luka/tests/`
3. **Test with both epoll (Linux) and kqueue (macOS)** if possible
4. **Test edge cases**: Large files, concurrent connections, malformed requests
5. **Memory leaks**: Use valgrind on Linux: `valgrind --leak-check=full ./build/webserv test.conf`

---

## Git Workflow

- Test configuration files are in `luka/conf/`
- Build artifacts are in `build/` (gitignored)
- Don't commit: `build/`, `*.o`, `*.a`, `*.d`, `server_log.txt`, `__pycache__/`

---

This guide ensures consistency and maintainability across the codebase. When in doubt, refer to existing code in similar components as examples.
