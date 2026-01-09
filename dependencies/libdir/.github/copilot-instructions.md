# Config Parser Library - AI Agent Instructions

## Architecture Overview

This is a **C++98 configuration parser library** implementing an **nginx-style configuration system** with automatic directive registration. All code lives in the `Config` namespace.

### Core Design Patterns

1. **Factory + Auto-Registration**: Directives register themselves at program startup via static initialization
2. **Composite Pattern**: Parent directives own and manage child directive lifetimes  
3. **Init Pattern**: String parsing (static `init` method) separated from typed construction
4. **Public Const Members**: Direct immutable access - NO getters, e.g., `listen->port`, `root->path`

### Component Structure

```
Config::Parser          → Tokenizes config files into directive strings
Config::IDirective      → Base class with composite pattern + RTTI getName()
Config::DirectiveFactory → Registry mapping names/types to creator functions
directives/             → 18 concrete directives (http, server, location, etc.)
Config::global          → Pre-allocated GlobalDirective* for simple usage
```

### Data Flow

```
.conf file → Parser::openConfigFile() → parser.getNextDirectives() 
→ IDirective::parseDirective() → Factory creates typed directive 
→ Parent owns child → Access via getFirstChild<T>() / getChildren<T>()
```

## Critical Patterns

### Creating New Directives

**ALWAYS** follow this exact pattern (see `directives/src/RootDirective.cpp`):

```cpp
namespace Config {
    // 1. Register with DEFINE_DIRECTIVE (lists allowed parent types)
    DEFINE_DIRECTIVE(RootDirective, "root", HttpDirective, ServerDirective, LocationDirective)
    
    // 2. Static init method parses string args → typed parameters
    RootDirective* RootDirective::initRootDirective(const std::vector<std::string>& args) {
        if (args.size() != 2) throw ArgumentException("root", "takes exactly one argument");
        return new RootDirective(args[1]); // Pass TYPED data
    }
    
    // 3. Typed constructor with const member initialization
    RootDirective::RootDirective(const std::string& path) 
        : IDirective(), path(path) {}  // path is const member
    
    // 4. Full OCF (Orthodox Canonical Form) required
    RootDirective::RootDirective(const RootDirective& other)
        : IDirective(other), path(other.path) {}
    RootDirective& operator=(const RootDirective& other) { /* path is const */ }
}
```

**Key Rules:**
- Init method is `static DirectiveType* initDirectiveName(args)` - returns new instance
- Constructor receives TYPED data (int, bool, vector<string>), NEVER raw strings
- Data members are `const` for immutability
- Must implement full OCF (default ctor, copy ctor, assignment, destructor)
- DEFINE_DIRECTIVE automatically creates factory registration

### Accessing Configuration

```cpp
// Wrap optional directives in try-catch
try {
    const Config::RootDirective* root = server->getFirstChild<Config::RootDirective>();
    std::string path = root->path;  // Direct access to const member
} catch (const Config::DirectiveNotFound&) {
    // Use default or inherit from parent
}

// Multiple instances (e.g., error_page, cgi_param)
std::vector<Config::ErrorPageDirective*> errors = 
    server->getChildren<Config::ErrorPageDirective>();
```

### Validation Patterns

**In init methods, validate args BEFORE calling constructor:**
- Argument count: `if (args.size() != expected) throw ArgumentException(...)`
- Enum values: Check against whitelist (e.g., HTTP methods: GET, POST, DELETE)
- Integer ranges: `if (code < 100 || code > 599) throw ArgumentException(...)`
- Boolean values: `if (value != "on" && value != "off") throw ArgumentException(...)`
- Extensions: Auto-add prefix if missing: `if (!ext.empty() && ext[0] != '.') ext = "." + ext`

## Build & Test Workflow

### Build Commands
```bash
make            # Build library → build/libdir.a
make re         # Clean rebuild
make clean      # Remove all build artifacts
```

### Debugging with Logging

The library includes a logging system with global log level control:

```cpp
#include "Logger.hpp"

// Set log level (defaults to LOG_LEVEL_WARN)
Config::g_log_level = Config::LOG_LEVEL_DEBUG;  // Show all messages
Config::g_log_level = Config::LOG_LEVEL_INFO;   // Info and above
Config::g_log_level = Config::LOG_LEVEL_WARN;   // Warnings and errors
Config::g_log_level = Config::LOG_LEVEL_ERROR;  // Errors only
Config::g_log_level = Config::LOG_LEVEL_NONE;   // Disable all logs

// Use macros for conditional logging
LOGE("Error message");               // Always shown (unless NONE)
LOGW("Warning about " << variable);  // Shown at WARN level and above
LOGI("Info: parsed " << count);      // Shown at INFO level and above
LOGD("Debug: " << details);          // Only at DEBUG level
```

**Key points:**
- Macros check `g_log_level` before output - zero overhead when disabled
- Use stream syntax: `LOGI("Port: " << port << " Host: " << host)`
- Set to `LOG_LEVEL_NONE` in production to eliminate all logging
- Set to `LOG_LEVEL_DEBUG` during development for verbose output

### Test Compilation Pattern
```bash
c++ -Wall -Wextra -Werror -std=c++98 -I./include -I./directives/include \
    tests/test_name.cpp build/libdir.a -o test_name
```

**CRITICAL**: Always use `-I./include -I./directives/include` (NO relative paths like `../../`)

### Running Tests
```bash
./test_webserv           # Comprehensive webserv config test
./test_all_directives    # All directive types
./test_ocf               # Orthodox Canonical Form
```

## Project-Specific Conventions

### Namespace Usage
- **All library code** in `namespace Config { }` 
- Headers: `#pragma once` (no include guards)
- Include directive headers: `#include "HttpDirective.hpp"` (clean paths via -I flags)

### Memory Management
- **Parent owns children**: `delete root` recursively deletes entire tree
- **Never delete children manually**: Owned by parent
- **Config::global**: Auto-managed GlobalDirective* (init on startup, cleanup on exit)

### Exception Hierarchy
```cpp
ConfigException                    // Base
├── FileException                  // File I/O errors
├── SyntaxException               // Parser syntax errors (line/column info)
├── ArgumentException             // Invalid directive arguments
├── DirectiveNotFound             // Missing required directive
└── (custom exceptions)
```

### Type Safety
- Template getters enforce compile-time type checking
- No dynamic_cast needed - use `getFirstChild<T>()` / `getChildren<T>()`
- RTTI provides getName() without virtual method overhead

## Integration Points

### For Webserv Project Usage
1. Link library: `build/libdir.a`
2. Parse config:
```cpp
Config::Parser parser = Config::Parser::openConfigFile("webserv.conf");
while (parser.getNextDirectives()) {
    Config::IDirective::parseDirective(Config::global, parser.copyDirective());
}
```
3. Extract servers:
```cpp
const Config::HttpDirective* http = Config::global->getFirstChild<Config::HttpDirective>();
std::vector<Config::ServerDirective*> servers = http->getChildren<Config::ServerDirective>();
```

### Adding to Makefile
When adding new directive, add BOTH files:
```makefile
FILES = ... \
    directives/src/NewDirective.cpp \  # Add to FILES variable
```

## Common Pitfalls

1. **DON'T** parse strings in constructor - do it in init method
2. **DON'T** use getters/setters - access public const members directly
3. **DON'T** forget DEFINE_DIRECTIVE - directive won't be registered
4. **DON'T** forget try-catch when accessing optional directives
5. **DON'T** use `args[0]` as data - it's the directive name
6. **DON'T** make data members non-const - breaks immutability contract
7. **DON'T** forget to update Makefile when adding new directive source file
8. **DON'T** use printf/cout directly - use LOGI/LOGE/LOGD macros for conditional output

## Documentation References

- `docs/CREATE_DIRECTIVE.md` - Step-by-step directive creation
- `docs/WEBSERV_DIRECTIVES.md` - All 18 built-in directives documented
- `docs/USAGE.md` - Library usage guide
- `tests/test_webserv.cpp` - Complete usage example
- `tests/webserv_complete.conf` - Full configuration example

## Quick Reference: 18 Built-in Directives

**Block**: http, server, location (with modifiers: =, ~, ~*, ^~, @)  
**Server**: server_name, listen  
**Paths**: root, index, autoindex  
**HTTP**: allowed_methods, return, error_page  
**Limits**: client_max_body_size, keepalive_timeout  
**Upload**: upload_enable, upload_store  
**CGI**: cgi_pass, cgi_param, cgi_extension
