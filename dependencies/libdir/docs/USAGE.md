# Configuration Parser Library - Usage Guide

This guide explains how to use the configuration parser library to read configuration files and access directive data.

## Table of Contents
- [Quick Start](#quick-start)
- [Reading Configuration Files](#reading-configuration-files)
- [Accessing Directive Data](#accessing-directive-data)
- [Working with Child Directives](#working-with-child-directives)
- [Error Handling](#error-handling)
- [Complete Examples](#complete-examples)
- [Best Practices](#best-practices)

---

## Quick Start

### Basic Usage Pattern

**Option 1: Using the Global Root (Recommended for Simple Cases)**

```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Exceptions.hpp"

using namespace Config;

int main() {
    try {
        // 1. Open and read configuration file
        Parser parser = Parser::openConfigFile("server.conf");
        
        // 2. Parse all directives into global root
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(Config::global, directive);
        }
        
        // 3. Access your configuration data from Config::global
        const ServerDirective* server = Config::global->getFirstChild<ServerDirective>();
        // ...
        
        // 4. No cleanup needed - Config::global is auto-managed
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

**Option 2: Managing Your Own Root**

```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "Exceptions.hpp"

using namespace Config;

int main() {
    try {
        // 1. Open and read configuration file
        Parser parser = Parser::openConfigFile("server.conf");
        
        // 2. Create root directive
        GlobalDirective* root = new GlobalDirective();
        
        // 3. Parse all directives
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(root, directive);
        }
        
        // 4. Access your configuration data
        // (see "Accessing Directive Data" section)
        
        // 5. Clean up
        delete root;  // Automatically deletes all children
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## Reading Configuration Files

### Method 1: Read from File

```cpp
try {
    // Opens file, reads content, creates parser
    Config::Parser parser = Config::Parser::openConfigFile("nginx.conf");
    
    GlobalDirective* root = new GlobalDirective();
    
    // Parse all directives
    while (parser.getNextDirectives()) {
        std::string directive = parser.copyDirective();
        IDirective::parseDirective(root, directive);
    }
    
    // Use root...
    delete root;
    
} catch (const FileException& e) {
    std::cerr << "Cannot open file: " << e.what() << std::endl;
}
```

### Method 2: Parse from String

```cpp
std::string configString = 
    "example value1 value2;\n"
    "example test data;\n";

Config::Parser parser(configString);
GlobalDirective* root = new GlobalDirective();

while (parser.getNextDirectives()) {
    std::string directive = parser.copyDirective();
    IDirective::parseDirective(root, directive);
}

// Use root...
delete root;
```

### Method 3: Using the Global Root

The library provides a global `Config::global` for convenience. This is a pre-allocated `GlobalDirective*` that is automatically initialized at program startup and cleaned up at program exit.

**When to use:**
- Simple applications with one configuration
- When you don't need multiple independent configuration trees
- When you want automatic memory management

**Usage:**
```cpp
#include "Config.hpp"

Config::Parser parser = Config::Parser::openConfigFile("server.conf");

while (parser.getNextDirectives()) {
    std::string directive = parser.copyDirective();
    IDirective::parseDirective(Config::global, directive);
}

// Access data
const ServerDirective* server = Config::global->getFirstChild<ServerDirective>();

// Config::global is automatically cleaned up at program exit
```

**Note:** The global instance is defined in `Config.cpp` and declared in `Config.hpp` as:
```cpp
namespace Config {
    extern Config::GlobalDirective *global;
}
```

---

## Accessing Directive Data

### Direct Member Access (Public Const Members)

Most directives expose their data as public const members for direct access:

```cpp
// Example directive with public const members
class ListenDirective : public IDirective {
public:
    const int port;
    const std::string host;
    // ...
};

// Usage:
const ListenDirective* listen = root->getFirstChild<ListenDirective>();
std::cout << "Port: " << listen->port << std::endl;
std::cout << "Host: " << listen->host << std::endl;
```

### Example: Reading ExampleDirective

```cpp
#include "Directives/ExampleDirective.hpp"

// Get first example directive
const ExampleDirective* example = root->getFirstChild<ExampleDirective>();

// Access data directly (public const members)
std::cout << "Arg1: " << example->arg1 << std::endl;
std::cout << "Arg2: " << example->arg2 << std::endl;
```

---

## Working with Child Directives

### Get Single Child

```cpp
// Get first child of specific type
const ServerDirective* server = root->getFirstChild<ServerDirective>();

// Get last child of specific type
const LocationDirective* location = server->getLastChild<LocationDirective>();
```

### Get All Children of Type

```cpp
// Get all children as vector
std::vector<ServerDirective*> servers = root->getChildren<ServerDirective>();

for (size_t i = 0; i < servers.size(); i++) {
    ServerDirective* server = servers[i];
    const ListenDirective* listen = server->getFirstChild<ListenDirective>();
    std::cout << "Server " << i << " on port: " << listen->port << std::endl;
}
```

### Get All Children (Any Type)

```cpp
// Get all children as multimap
std::multimap<std::string, IDirective*> children = root->getChildren();

std::multimap<std::string, IDirective*>::iterator it;
for (it = children.begin(); it != children.end(); ++it) {
    std::string name = it->first;
    IDirective* child = it->second;
    std::cout << "Found directive: " << name << std::endl;
}
```

### Using ListDirectives Wrapper

```cpp
// Get children wrapped in convenient container
ListDirectives<ServerDirective> servers = root->getMany<ServerDirective>();

for (size_t i = 0; i < servers.size(); i++) {
    const ServerDirective* server = servers[i];
    const ListenDirective* listen = server->getFirstChild<ListenDirective>();
    std::cout << "Server port: " << listen->port << std::endl;
}
```

### Nested Directive Access

```cpp
// Access nested directives
const ServerDirective* server = root->getFirstChild<ServerDirective>();
const LocationDirective* location = server->getFirstChild<LocationDirective>();
const ProxyPassDirective* proxy = location->getFirstChild<ProxyPassDirective>();

std::cout << "Proxy to: " << proxy->host << ":" << proxy->port << std::endl;
```

---

## Error Handling

### Exception Hierarchy

```
std::exception
    └── ConfigException           // Base for all config errors
            ├── FileException           // File I/O errors
            ├── ParserException         // Parsing/syntax errors
            │       └── SyntaxException     // Specific syntax errors
            ├── DirectiveException      // Directive errors
            │       ├── DirectiveNotFound       // Unknown directive
            │       ├── DirectiveTypeMismatch   // Type casting failed
            │       ├── InvalidNesting          // Wrong parent context
            │       └── ArgumentException       // Invalid arguments
            └── ValidationException     // Validation errors
```

### Catching Exceptions

```cpp
try {
    Config::Parser parser = Config::Parser::openConfigFile("server.conf");
    GlobalDirective* root = new GlobalDirective();
    
    while (parser.getNextDirectives()) {
        std::string directive = parser.copyDirective();
        IDirective::parseDirective(root, directive);
    }
    
    // Use configuration...
    
    delete root;
    
} catch (const FileException& e) {
    std::cerr << "File error: " << e.what() << std::endl;
    std::cerr << "File: " << e.getFilename() << std::endl;
    
} catch (const SyntaxException& e) {
    std::cerr << "Syntax error: " << e.what() << std::endl;
    std::cerr << "Line: " << e.getLine() << ", Column: " << e.getColumn() << std::endl;
    
} catch (const ArgumentException& e) {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
    std::cerr << "Directive: " << e.getDirectiveName() << std::endl;
    
} catch (const InvalidNesting& e) {
    std::cerr << "Invalid nesting: " << e.what() << std::endl;
    std::cerr << "Cannot place " << e.getChildType() 
              << " inside " << e.getParentType() << std::endl;
    
} catch (const DirectiveNotFound& e) {
    std::cerr << "Directive not found: " << e.what() << std::endl;
    
} catch (const ConfigException& e) {
    std::cerr << "Configuration error: " << e.what() << std::endl;
}
```

### Checking for Optional Directives

```cpp
try {
    const ListenDirective* listen = root->getFirstChild<ListenDirective>();
    // Directive exists, use it
    std::cout << "Port: " << listen->port << std::endl;
    
} catch (const DirectiveNotFound& e) {
    // Directive doesn't exist, use default
    std::cout << "Using default port: 8080" << std::endl;
}
```

---

## Complete Examples

### Example 1: Simple Server Configuration

**Configuration file (server.conf):**
```nginx
listen 8080 localhost;
listen 443 0.0.0.0;
timeout 30;
```

**C++ code:**
```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "Directives/ListenDirective.hpp"
#include "Directives/TimeoutDirective.hpp"
#include "Exceptions.hpp"

using namespace Config;

int main() {
    try {
        // Parse configuration
        Parser parser = Parser::openConfigFile("server.conf");
        GlobalDirective* root = new GlobalDirective();
        
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(root, directive);
        }
        
        // Get all listen directives
        std::vector<ListenDirective*> listeners = root->getChildren<ListenDirective>();
        
        std::cout << "Found " << listeners.size() << " listen directive(s):" << std::endl;
        for (size_t i = 0; i < listeners.size(); i++) {
            std::cout << "  - Port " << listeners[i]->port 
                      << " on " << listeners[i]->host << std::endl;
        }
        
        // Get timeout
        try {
            const TimeoutDirective* timeout = root->getFirstChild<TimeoutDirective>();
            std::cout << "Timeout: " << timeout->seconds << " seconds" << std::endl;
        } catch (const DirectiveNotFound& e) {
            std::cout << "No timeout specified, using default" << std::endl;
        }
        
        delete root;
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Example 2: Nested Configuration (Server with Locations)

**Configuration file (nginx.conf):**
```nginx
server {
    listen 8080;
    
    location /api {
        proxy_pass http://backend:3000;
    }
    
    location /static {
        root /var/www;
    }
}
```

**C++ code:**
```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "Directives/ServerDirective.hpp"
#include "Directives/LocationDirective.hpp"
#include "Directives/ProxyPassDirective.hpp"
#include "Exceptions.hpp"

using namespace Config;

int main() {
    try {
        // Parse configuration
        Parser parser = Parser::openConfigFile("nginx.conf");
        GlobalDirective* root = new GlobalDirective();
        
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(root, directive);
        }
        
        // Get all servers
        std::vector<ServerDirective*> servers = root->getChildren<ServerDirective>();
        
        for (size_t i = 0; i < servers.size(); i++) {
            ServerDirective* server = servers[i];
            std::cout << "Server " << i << ":" << std::endl;
            
            // Get listen directive
            const ListenDirective* listen = server->getFirstChild<ListenDirective>();
            std::cout << "  Listening on port: " << listen->port << std::endl;
            
            // Get all locations
            std::vector<LocationDirective*> locations = 
                server->getChildren<LocationDirective>();
            
            std::cout << "  Locations:" << std::endl;
            for (size_t j = 0; j < locations.size(); j++) {
                LocationDirective* loc = locations[j];
                std::cout << "    - " << loc->path << std::endl;
                
                // Check for proxy_pass
                try {
                    const ProxyPassDirective* proxy = 
                        loc->getFirstChild<ProxyPassDirective>();
                    std::cout << "      Proxies to: " << proxy->scheme 
                              << "://" << proxy->host << ":" << proxy->port 
                              << std::endl;
                } catch (const DirectiveNotFound& e) {
                    // No proxy_pass, might have root or other directives
                }
            }
        }
        
        delete root;
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Example 3: Iterating Over All Directives

```cpp
void printDirectiveTree(const IDirective* directive, int indent = 0) {
    // Print indent
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
    
    // Print directive name
    std::cout << "- " << directive->getName() << std::endl;
    
    // Print all children recursively
    std::multimap<std::string, IDirective*> children = directive->getChildren();
    std::multimap<std::string, IDirective*>::const_iterator it;
    
    for (it = children.begin(); it != children.end(); ++it) {
        printDirectiveTree(it->second, indent + 1);
    }
}

int main() {
    try {
        Config::Parser parser = Config::Parser::openConfigFile("nginx.conf");
        GlobalDirective* root = new GlobalDirective();
        
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            IDirective::parseDirective(root, directive);
        }
        
        std::cout << "Configuration tree:" << std::endl;
        printDirectiveTree(root);
        
        delete root;
        
    } catch (const ConfigException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## Best Practices

### ✅ Do's

1. **Always use try-catch for error handling**
   ```cpp
   try {
       // Parse configuration
   } catch (const ConfigException& e) {
       // Handle errors
   }
   ```

2. **Check for optional directives**
   ```cpp
   try {
       const TimeoutDirective* timeout = root->getFirstChild<TimeoutDirective>();
       // Use timeout
   } catch (const DirectiveNotFound& e) {
       // Use default value
   }
   ```

3. **Manage memory properly**
   ```cpp
   // Option A: Use Config::global (auto-managed)
   IDirective::parseDirective(Config::global, directive);
   // No cleanup needed
   
   // Option B: Manage your own root
   GlobalDirective* root = new GlobalDirective();
   // ... use root ...
   delete root;  // Cleans up all children
   ```

4. **Access data via public const members**
   ```cpp
   std::cout << directive->port << std::endl;  // Direct access
   ```

5. **Use type-safe getters**
   ```cpp
   const ServerDirective* server = root->getFirstChild<ServerDirective>();
   ```

### ❌ Don'ts

1. **Don't forget to delete the root directive**
   ```cpp
   // BAD - memory leak!
   GlobalDirective* root = new GlobalDirective();
   // ... use root but forget to delete
   ```

2. **Don't manually delete child directives**
   ```cpp
   // BAD - children are owned by parent!
   const ServerDirective* server = root->getFirstChild<ServerDirective>();
   delete server;  // NO! Parent owns this
   ```

3. **Don't ignore exceptions**
   ```cpp
   // BAD - no error handling!
   Parser parser = Parser::openConfigFile("file.conf");
   // What if file doesn't exist?
   ```

4. **Don't use dynamic_cast directly**
   ```cpp
   // BAD - use template methods instead
   IDirective* child = root->getChildren().begin()->second;
   ServerDirective* server = dynamic_cast<ServerDirective*>(child);
   
   // GOOD - type-safe template method
   const ServerDirective* server = root->getFirstChild<ServerDirective>();
   ```

5. **Don't modify const members after construction**
   ```cpp
   // Const members are immutable by design!
   // This won't compile:
   // directive->port = 9090;  // ERROR!
   ```

---

## Configuration File Syntax

### Comments
```nginx
# This is a comment
listen 8080;  # Inline comment
```

### Simple Directives
```nginx
listen 8080 localhost;
timeout 30;
root /var/www;
```

### Block Directives
```nginx
server {
    listen 8080;
    
    location /api {
        proxy_pass http://backend:3000;
    }
}
```

### Quoted Strings
```nginx
# Double quotes
root "/path/with spaces";

# Single quotes
name 'O\'Reilly';

# Escape sequences
path "C:\\Program Files\\App";
```

### Semicolons
- Required at end of simple directives
- Not required for block directives (use `}`)

---

## API Reference Summary

### Parser Methods
- `Parser(std::string config)` - Parse from string
- `Parser::openConfigFile(const char* filename)` - Parse from file
- `bool getNextDirectives()` - Get next directive (returns false at end)
- `std::string copyDirective()` - Get current directive text

### IDirective Methods
- `static const IDirective* parseDirective(IDirective* parent, const std::string& str)` - Parse and add directive
- `std::multimap<std::string, IDirective*> getChildren()` - Get all children
- `std::vector<T*> getChildren<T>()` - Get children of specific type
- `const T* getFirstChild<T>()` - Get first child of type
- `const T* getLastChild<T>()` - Get last child of type
- `std::string getName()` - Get directive name

### Global Variable
- `Config::global` - Pre-allocated `GlobalDirective*` (auto-managed)
  - Automatically initialized at program startup
  - Automatically cleaned up at program exit
  - Use for simple applications or single configuration trees
  - Declared in `Config.hpp`, defined in `Config.cpp`

---

## Need Help?

- **Creating new directives?** See [CREATE_DIRECTIVE.md](CREATE_DIRECTIVE.md)
- **Library improvements?** See [IMPROVEMENTS.md](IMPROVEMENTS.md)
- **Example code?** Check `test_improvements.cpp` and `test_ocf.cpp`
- **Implementation details?** See `OCF_IMPLEMENTATION.md`

---

## Summary

**Reading configuration:**
1. Create `Parser` from file or string
2. Create `GlobalDirective` root
3. Loop with `getNextDirectives()` and `parseDirective()`

**Accessing data:**
1. Use `getFirstChild<T>()` or `getChildren<T>()`
2. Access public const members directly
3. Handle `DirectiveNotFound` for optional directives

**Error handling:**
1. Wrap everything in try-catch
2. Catch specific exceptions for detailed errors
3. Catch `ConfigException` as fallback

**Memory management:**
1. Create root with `new`
2. Parser and `parseDirective()` handle children
3. Delete root when done (cleans up everything)
