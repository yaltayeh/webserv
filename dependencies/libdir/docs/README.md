# Configuration Parser Library

A C++98 configuration parser library with Nginx-style syntax support, featuring automatic directive registration, type-safe access, and comprehensive error handling.

## Features

- **Nginx-style configuration syntax** - Familiar, hierarchical configuration format
- **Automatic directive registration** - No manual factory setup required
- **Type-safe directive access** - Template-based getters with compile-time safety
- **Separation of concerns** - String parsing separated from object construction
- **Public const members** - Direct, immutable access to configuration data
- **Comprehensive error handling** - Detailed exception hierarchy for debugging
- **Memory-safe** - Automatic cleanup with RAII and parent ownership
- **C++98 compliant** - Works with older compilers

## Quick Start

```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "Exceptions.hpp"

using namespace Config;

int main() {
    try {
        // Parse configuration file
        Parser parser = Parser::openConfigFile("server.conf");
        GlobalDirective* root = new GlobalDirective();
        
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(root, directive);
        }
        
        // Access configuration data
        const ServerDirective* server = root->getFirstChild<ServerDirective>();
        const ListenDirective* listen = server->getFirstChild<ListenDirective>();
        std::cout << "Port: " << listen->port << std::endl;
        
        delete root;  // Automatic cleanup
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Configuration File Example

```nginx
# Server configuration
server {
    listen 8080 localhost;
    timeout 30;
    
    location /api {
        proxy_pass http://backend:3000;
    }
    
    location /static {
        root /var/www;
    }
}
```

## Documentation

- **[USAGE.md](USAGE.md)** - Complete usage guide with examples
  - Reading configuration files
  - Accessing directive data
  - Working with child directives
  - Error handling
  - Complete examples

- **[CREATE_DIRECTIVE.md](CREATE_DIRECTIVE.md)** - Guide for creating custom directives
  - Step-by-step directive creation
  - Init pattern explanation
  - Exception handling in directives
  - Best practices

- **[IMPROVEMENTS.md](IMPROVEMENTS.md)** - Library improvements and enhancements
  - Escape sequences
  - Exception handling
  - Memory management
  - OCF implementation

- **[OCF_IMPLEMENTATION.md](OCF_IMPLEMENTATION.md)** - Orthodox Canonical Form details

## Building

```bash
# Build the library
make clean && make

# The library is created at: build/libdir.a
```

## Testing

```bash
# Compile tests with force_load to include static registrations
c++ -Wall -Wextra -Werror -std=c++98 -I./include \
    -o test_example test_example.cpp \
    -Lbuild -Wl,-force_load,./build/libdir.a

# Run tests
./test_argument_exception
./test_ocf
./test_improvements
```

## Library Structure

```
include/
├── Config.hpp              # Global root directive
├── Parser.hpp              # Configuration file parser
├── IDirective.hpp          # Base directive class
├── DirectiveFactory.hpp    # Factory and registration system
├── Exceptions.hpp          # Exception hierarchy
├── ListDirectives.hpp      # Container for directives
└── Directives/
    ├── GlobalDirective.hpp # Root directive
    └── ExampleDirective.hpp # Example implementation

src/
├── Config.cpp
├── Parser.cpp
├── IDirective.cpp
├── DirectiveFactory.cpp
└── Directives/
    ├── GlobalDirective.cpp
    └── ExampleDirective.cpp
```

## Core Concepts

### 1. Parser
Reads configuration files and tokenizes them into directive strings.

### 2. Directives
Configuration elements that inherit from `IDirective`. Each directive:
- Has a typed constructor for strong typing
- Has a static `init` method for string parsing
- Uses public const members for direct data access
- Is automatically registered via `DEFINE_DIRECTIVE` macro

### 3. Factory Pattern
Directives are automatically registered at startup and created dynamically by name.

### 4. Composite Pattern
Directives form a tree structure where parents own their children.

### 5. Init Pattern
Separation of string parsing (init method) from object construction (constructor) for clean, type-safe code.

## Example: Creating a Custom Directive

**Header (ListenDirective.hpp):**
```cpp
#include "../IDirective.hpp"
#pragma once

namespace Config {

class ListenDirective : public IDirective {
public:
    const int port;
    const std::string host;

public:
    ListenDirective();
    ListenDirective(int port, const std::string& host);
    ListenDirective(const ListenDirective& other);
    ListenDirective& operator=(const ListenDirective& other);
    ~ListenDirective();
    
    static ListenDirective* initListenDirective(const std::vector<std::string>& args);
    DEFINE_ALLOWED_INSIDE();
};

} // namespace Config
```

**Implementation (ListenDirective.cpp):**
```cpp
#include "Directives/ListenDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <cstdlib>

namespace Config {

DEFINE_DIRECTIVE(ListenDirective, "listen", ServerDirective, GlobalDirective)

ListenDirective* ListenDirective::initListenDirective(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw ArgumentException("listen", "requires at least 1 argument (port)");
    }
    
    int port = std::atoi(args[1].c_str());
    if (port < 1 || port > 65535) {
        throw ArgumentException("listen", "port must be between 1-65535");
    }
    
    std::string host = args.size() > 2 ? args[2] : "0.0.0.0";
    return new ListenDirective(port, host);
}

ListenDirective::ListenDirective() : port(8080), host("0.0.0.0") {}

ListenDirective::ListenDirective(int port, const std::string& host)
    : port(port), host(host) {}

// Copy constructor, assignment operator, destructor...

} // namespace Config
```

**Usage:**
```cpp
const ListenDirective* listen = root->getFirstChild<ListenDirective>();
std::cout << "Port: " << listen->port << std::endl;
std::cout << "Host: " << listen->host << std::endl;
```

## API Overview

### Reading Configuration
```cpp
// From file
Parser parser = Parser::openConfigFile("file.conf");

// From string
Parser parser(configString);

// Parse directives
while (parser.getNextDirectives()) {
    IDirective::parseDirective(root, parser.copyDirective());
}
```

### Accessing Data
```cpp
// Get first child of type
const ServerDirective* server = root->getFirstChild<ServerDirective>();

// Get all children of type
std::vector<LocationDirective*> locations = server->getChildren<LocationDirective>();

// Access data directly (get port from listen directive)
const ListenDirective* listen = server->getFirstChild<ListenDirective>();
std::cout << listen->port << std::endl;
```

### Error Handling
```cpp
try {
    // Parse configuration
} catch (const FileException& e) {
    // File not found
} catch (const ArgumentException& e) {
    // Invalid directive arguments
} catch (const ConfigException& e) {
    // General configuration error
}
```

## Requirements

- C++98 or later
- Standard C++ library
- Make (for building)

## License

[Your license here]

## Contributing

[Contributing guidelines here]

## Support

For detailed documentation:
- Read [USAGE.md](USAGE.md) for usage examples
- Read [CREATE_DIRECTIVE.md](CREATE_DIRECTIVE.md) for creating custom directives
- Check test files for working examples
