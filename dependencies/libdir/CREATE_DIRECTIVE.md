# How to Create a New Directive

This guide explains step-by-step how to create a custom directive for the configuration parser library.

## Table of Contents
- [Overview](#overview)
- [Quick Start](#quick-start)
- [Step-by-Step Guide](#step-by-step-guide)
- [Examples](#examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

---

## Overview

The directive system uses a **Factory Pattern** with automatic registration. Each directive:
- Inherits from `IDirective`
- Has typed constructor parameters (not raw strings!)
- Implements a static `init` method for parsing string arguments
- Specifies which parent directives can contain it
- Is automatically registered before `main()` runs

**Key Components:**
- **Header file** (`.hpp`): Class declaration
- **Implementation file** (`.cpp`): Static init method + constructor
- **DEFINE_DIRECTIVE macro**: Automatic registration and factory setup

---

## Quick Start

For a directive named `listen` with port and host parameters:

**1. Create `include/Directives/ListenDirective.hpp`:**
```cpp
#include "Directives.hpp"
#pragma once

class ListenDirective : public IDirective
{
public:
    const int port;
    const std::string host;

public:
    ListenDirective();
    ListenDirective(int port, const std::string& host);
    ListenDirective(const ListenDirective &other);
    ListenDirective &operator=(const ListenDirective &other);
    ~ListenDirective();
    
    static ListenDirective* initListenDirective(const std::vector<std::string>& args);
    DEFINE_ALLOWED_INSIDE();
};
```

**2. Create `src/Directives/ListenDirective.cpp`:**
```cpp
#include "Directives/ListenDirective.hpp"
#include "DirectiveFactory.hpp"
#include <cstdlib>

DEFINE_DIRECTIVE(ListenDirective, "listen", ServerDirective, GlobalDirective)

ListenDirective* ListenDirective::initListenDirective(const std::vector<std::string>& args)
{
    // args[0] = "listen"
    // args[1] = port number
    // args[2] = host (optional)
    
    int port = std::atoi(args[1].c_str());
    std::string host = args.size() > 2 ? args[2] : "0.0.0.0";
    
    return new ListenDirective(port, host);
}

ListenDirective::ListenDirective() : port(8080), host("0.0.0.0") {}

ListenDirective::ListenDirective(int port, const std::string& host)
    : port(port), host(host) {}

// Copy constructor and other OCF methods...
```

**3. Add to Makefile and rebuild:**
```makefile
FILES = ... \
        src/Directives/ListenDirective.cpp
```

**4. Use in configuration:**
```nginx
server {
    listen 8080 localhost;
    listen 443 0.0.0.0;
}
```

---

## Step-by-Step Guide

### Step 1: Design Your Directive

**Decide on:**
1. **Directive name**: How it appears in config files (e.g., "listen", "server", "location")
2. **Parameters**: What data it needs (port, path, timeout, etc.)
3. **Parameter types**: int, string, bool, etc.
4. **Parent contexts**: Where can this directive appear? (root level, inside server, etc.)

**Example Design:**
```
Directive: listen
Parameters: 
  - port (int, required)
  - host (string, optional, default: "0.0.0.0")
Parent contexts: ServerDirective, GlobalDirective
```

---

### Step 2: Create the Header File

**Location:** `include/Directives/YourDirective.hpp`

**Required Elements:**

```cpp
#include "Directives.hpp"  // Provides IDirective base class
#pragma once

/**
 * @class YourDirective
 * @brief Description of what this directive does
 */
class YourDirective : public IDirective
{
public:
    // 1. Declare member variables as public const (immutable and directly accessible)
    const int someNumber;
    const std::string someText;

public:
    // 2. Orthodox Canonical Form (OCF) - Required!
    YourDirective();  // Default constructor
    YourDirective(int num, const std::string& text);  // Typed constructor
    YourDirective(const YourDirective &other);  // Copy constructor
    YourDirective &operator=(const YourDirective &other);  // Assignment
    ~YourDirective();  // Destructor
    
    // 3. Static init method - CRITICAL!
    static YourDirective* initYourDirective(const std::vector<std::string>& args);
    
    // 4. Declare getAllowedInside() method
    DEFINE_ALLOWED_INSIDE();
};
```

**Key Points:**
- ✅ Use `const` for members that shouldn't change after construction
- ✅ Constructor takes **typed parameters**, not `vector<string>`
- ✅ Static init method name MUST be `init##ClassName` (e.g., `initYourDirective`)
- ✅ Include all OCF methods (copy constructor, assignment, etc.)

---

### Step 3: Create the Implementation File

**Location:** `src/Directives/YourDirective.cpp`

**Required Elements:**

```cpp
#include "Directives/YourDirective.hpp"
#include "Directives/ServerDirective.hpp"  // Include parent directive headers
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"  // For ArgumentException
#include <cstdlib>  // For atoi, atof, etc.

// 1. DEFINE_DIRECTIVE macro - MUST be at the top!
//    Format: DEFINE_DIRECTIVE(ClassName, "config-name", Parent1, Parent2, ...)
DEFINE_DIRECTIVE(YourDirective, "yourdirective", ServerDirective, GlobalDirective)

// 2. Implement the static init method - this is where string parsing happens
YourDirective* YourDirective::initYourDirective(const std::vector<std::string>& args)
{
    // args[0] is always the directive name
    // args[1+] are your actual parameters
    
    // Parse and validate arguments
    if (args.size() < 2) {
        throw ArgumentException("yourdirective", "requires at least 1 argument");
    }
    
    // Convert strings to proper types
    int num = std::atoi(args[1].c_str());
    std::string text = args.size() > 2 ? args[2] : "default";
    
    // Validate if needed
    if (num < 0) {
        throw ArgumentException("yourdirective", "number must be positive, got " + args[1]);
    }
    
    // Call typed constructor
    return new YourDirective(num, text);
}

**Best practices for init methods:**
- Do all string→type conversion in init
- Do all validation in init using ArgumentException
- Keep constructor simple (just initialize members)
- Return `new YourDirective(...)` at the end

### Exception Handling in Init Methods

When validating arguments in your `init` method, use **`ArgumentException`** for argument-related errors:

```cpp
#include "Exceptions.hpp"

YourDirective* YourDirective::initYourDirective(const std::vector<std::string>& args)
{
    // Check argument count
    if (args.size() != 2) {
        throw ArgumentException("yourdirective", "requires exactly 1 argument");
    }
    
    // Validate argument format/value
    int port = std::atoi(args[1].c_str());
    if (port < 1 || port > 65535) {
        throw ArgumentException("yourdirective", 
            "port must be between 1-65535, got " + args[1]);
    }
    
    return new YourDirective(port);
}
```

**Exception Hierarchy:**
```
std::exception
    └── ConfigException         // Base for all config errors
            ├── ParserException     // Syntax errors (missing {}, etc.)
            ├── DirectiveException  // Directive-related errors
            │       └── ArgumentException  // ← Use this in init methods
            └── ValidationException // Config validation errors
```

**Where Exceptions Are Caught:**

1. **Parser Level** (automatic):
   - `Parser::parseDirective()` catches exceptions during directive creation
   - Exceptions are automatically propagated up with file/line context
   - You don't need to catch in your init method

2. **Application Level** (optional):
   ```cpp
   try {
       Config config("nginx.conf");
   } catch (const ArgumentException& e) {
       std::cerr << "Invalid directive argument: " << e.what() << std::endl;
       std::cerr << "Directive: " << e.getDirectiveName() << std::endl;
   } catch (const ConfigException& e) {
       std::cerr << "Configuration error: " << e.what() << std::endl;
   }
   ```

**When to use each exception:**
- **`ArgumentException`**: Wrong number of arguments, invalid values, type conversion failures
- **`DirectiveException`**: Directive used in wrong context (parent validation)
- **`ValidationException`**: Cross-directive validation (e.g., conflicting settings)
- **`ParserException`**: Never throw this - parser uses it for syntax errors

// 3. Implement default constructor
YourDirective::YourDirective() : someNumber(0), someText("default") {}

// 4. Implement typed constructor
YourDirective::YourDirective(int num, const std::string& text)
    : someNumber(num), someText(text)
{
    // Constructor logic here (if needed)
}

// 5. Implement OCF methods
YourDirective::YourDirective(const YourDirective &other) 
    : IDirective(other), someNumber(other.someNumber), someText(other.someText) {}

YourDirective &YourDirective::operator=(const YourDirective &other)
{
    if (this == &other)
        return *this;
    IDirective::operator=(other);
    // Cannot reassign const members - limitation of const + OCF
    return *this;
}

YourDirective::~YourDirective() {}
```

**Init Method Best Practices:**
- ✅ Always check `args.size()` before accessing indices
- ✅ Provide default values for optional parameters
- ✅ Throw exceptions for invalid input (caught by parser)
- ✅ Use appropriate conversion functions (`atoi`, `atof`, custom parsers)
- ✅ Keep parsing logic here, not in constructor

---

### Step 4: Add to Build System

**Edit `Makefile`:**

```makefile
FILES = src/Parser.cpp \
        src/IDirective.cpp \
        src/DirectiveFactory.cpp \
        src/Config.cpp \
        src/Directives/GlobalDirective.cpp \
        src/Directives/ExampleDirective.cpp \
        src/Directives/YourDirective.cpp \    # <-- Add your file here
```

---

### Step 5: Build and Test

```bash
# Build the library
make clean && make

# Test in code
GlobalDirective root;
const char* config = "yourdirective 42 hello;";
IDirective::parseDirective(&root, config);
```

---

## Examples

### Example 1: Simple Directive with Single Parameter

**Configuration:**
```nginx
timeout 30;
```

**Header (TimeoutDirective.hpp):**
```cpp
class TimeoutDirective : public IDirective
{
public:
    const int seconds;

public:
    TimeoutDirective();
    TimeoutDirective(int seconds);
    // ... OCF methods ...
    static TimeoutDirective* initTimeoutDirective(const std::vector<std::string>& args);
    DEFINE_ALLOWED_INSIDE();
};
```

**Implementation (TimeoutDirective.cpp):**
```cpp
DEFINE_DIRECTIVE(TimeoutDirective, "timeout", ServerDirective, GlobalDirective)

TimeoutDirective* TimeoutDirective::initTimeoutDirective(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        throw ArgumentException("timeout", "requires a value");
    }
    
    int seconds = std::atoi(args[1].c_str());
    
    if (seconds <= 0) {
        throw ArgumentException("timeout", "must be positive, got " + args[1]);
    }
    
    return new TimeoutDirective(seconds);
}

TimeoutDirective::TimeoutDirective() : seconds(60) {}
TimeoutDirective::TimeoutDirective(int seconds) : seconds(seconds) {}
```

---

### Example 2: Directive with Multiple Parameters

**Configuration:**
```nginx
proxy_pass http://backend:8080 /api;
```

**Header:**
```cpp
class ProxyPassDirective : public IDirective
{
public:
    const std::string scheme;
    const std::string host;
    const int port;
    const std::string path;

public:
    ProxyPassDirective();
    ProxyPassDirective(const std::string& scheme, const std::string& host, 
                       int port, const std::string& path);
    // ... OCF methods ...
    static ProxyPassDirective* initProxyPassDirective(const std::vector<std::string>& args);
    DEFINE_ALLOWED_INSIDE();
};
```

**Implementation:**
```cpp
DEFINE_DIRECTIVE(ProxyPassDirective, "proxy_pass", LocationDirective)

ProxyPassDirective* ProxyPassDirective::initProxyPassDirective(const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        throw ArgumentException("proxy_pass", "requires a URL");
    }
    
    // Parse URL: "http://backend:8080"
    std::string url = args[1];
    
    // Simple parsing (you'd want more robust parsing in production)
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        throw ArgumentException("proxy_pass", "invalid URL format: " + url);
    }
    
    std::string scheme = url.substr(0, schemeEnd);
    
    size_t hostStart = schemeEnd + 3;
    size_t portStart = url.find(":", hostStart);
    if (portStart == std::string::npos) {
        throw ArgumentException("proxy_pass", "port not specified in URL: " + url);
    }
    
    std::string host = url.substr(hostStart, portStart - hostStart);
    
    int port = std::atoi(url.substr(portStart + 1).c_str());
    if (port <= 0 || port > 65535) {
        throw ArgumentException("proxy_pass", "invalid port in URL: " + url);
    }
    
    std::string path = args.size() > 2 ? args[2] : "/";
    
    return new ProxyPassDirective(scheme, host, port, path);
}
```

---

### Example 3: Boolean Flag Directive

**Configuration:**
```nginx
enable_compression on;
enable_compression off;
```

**Header:**
```cpp
class EnableCompressionDirective : public IDirective
{
public:
    const bool enabled;

public:
    EnableCompressionDirective();
    EnableCompressionDirective(bool enabled);
    // ... OCF methods ...
    static EnableCompressionDirective* initEnableCompressionDirective(const std::vector<std::string>& args);
    DEFINE_ALLOWED_INSIDE();
};
```

**Implementation:**
```cpp
DEFINE_DIRECTIVE(EnableCompressionDirective, "enable_compression", ServerDirective)

EnableCompressionDirective* EnableCompressionDirective::initEnableCompressionDirective(
    const std::vector<std::string>& args)
{
    if (args.size() < 2) {
        throw ArgumentException("enable_compression", "requires on/off value");
    }
    
    std::string value = args[1];
    bool enabled;
    
    if (value == "on" || value == "true" || value == "1") {
        enabled = true;
    } else if (value == "off" || value == "false" || value == "0") {
        enabled = false;
    } else {
        throw ArgumentException("enable_compression", 
            "invalid value '" + value + "' (use on/off)");
    }
    
    return new EnableCompressionDirective(enabled);
}
```

---

## Best Practices

### ✅ Do's

1. **Use public const for immutable members**
   ```cpp
   public:
       const int port;  // Immutable and directly accessible
       const std::string host;
   ```
   Since they're const, they can't be modified, so making them public is safe and convenient.
   
   **Usage:**
   ```cpp
   ListenDirective* listen = ...
   std::cout << "Port: " << listen->port << std::endl;  // Direct access!
   ```

2. **Validate arguments using ArgumentException**
   ```cpp
   if (port < 1 || port > 65535) {
       throw ArgumentException("listen", "invalid port number: " + args[1]);
   }
   ```

3. **Provide sensible defaults**
   ```cpp
   std::string host = args.size() > 2 ? args[2] : "0.0.0.0";
   ```

4. **Check argument count first**
   ```cpp
   if (args.size() < 2) {
       throw ArgumentException("listen", "requires at least 1 argument");
   }
   ```

5. **Use initialization lists**
   ```cpp
   MyDirective::MyDirective(int port) : port(port) {}
   ```

6. **Document your directive**
   ```cpp
   /**
    * @brief Listen directive for server configuration
    * @param port Port number to listen on
    * @param host Host address (default: 0.0.0.0)
    */
   ```

### ❌ Don'ts

1. **Don't parse in constructor**
   ```cpp
   // BAD - parsing string arguments in constructor
   MyDirective(const std::vector<std::string>& args) {
       port = std::atoi(args[1].c_str());  // NO!
   }
   ```

2. **Don't forget DEFINE_DIRECTIVE**
   ```cpp
   // This MUST be in your .cpp file!
   DEFINE_DIRECTIVE(MyDirective, "mydirective", ParentDirective)
   ```

3. **Don't use non-const members without reason**
   ```cpp
   int port;  // Can be modified - is this intended?
   ```

4. **Don't forget OCF methods**
   - Copy constructor, assignment operator, destructor are required

5. **Don't access args without bounds checking**
   ```cpp
   // BAD
5. **Don't silently ignore errors**
   ```cpp
   // BAD
   int port = std::atoi(args[1].c_str());  // What if args.size() == 1?
   
   // GOOD
   if (args.size() < 2) {
       throw ArgumentException("listen", "missing port argument");
   }
   int port = std::atoi(args[1].c_str());
   ```

---

## Troubleshooting

### Error: "undefined reference to initMyDirective"

**Cause:** Missing implementation of static init method

**Solution:** Make sure you implemented `initMyDirective` in your .cpp file:
```cpp
MyDirective* MyDirective::initMyDirective(const std::vector<std::string>& args) {
    // implementation
}
```

---

### Error: "no matching function for call to MyDirective::MyDirective"

**Cause:** DEFINE_DIRECTIVE is calling init method which tries to call constructor, but constructor signature doesn't match

**Solution:** Make sure your constructor accepts the types you're passing from init:
```cpp
// In init method:
return new MyDirective(port, host);  // Passing int and string

// Constructor must match:
MyDirective(int port, const std::string& host);  // ✅ Correct
```

---

### Error: Directive not found at runtime

**Cause:** DEFINE_DIRECTIVE not called, or .cpp file not linked

**Solution:**
1. Check that DEFINE_DIRECTIVE is at the top of your .cpp file
2. Verify your .cpp is in the Makefile:
   ```makefile
   FILES = ... src/Directives/MyDirective.cpp
   ```
3. Rebuild: `make clean && make`

---

### Directive appears in wrong context

**Cause:** Parent directives not correctly specified in DEFINE_DIRECTIVE

**Solution:** Check the parent list in DEFINE_DIRECTIVE:
```cpp
// Can only appear inside ServerDirective
DEFINE_DIRECTIVE(ListenDirective, "listen", ServerDirective)

// Can appear in multiple contexts
DEFINE_DIRECTIVE(TimeoutDirective, "timeout", ServerDirective, LocationDirective, GlobalDirective)
```

---

## Summary

**Creating a new directive requires:**

1. ✅ Header file with class declaration, typed constructor, and static init method
2. ✅ Implementation file with DEFINE_DIRECTIVE, init method, and constructors
3. ✅ Add to Makefile and rebuild
4. ✅ Test your directive

**The pattern:**
```
Config file → Parser → Factory → init method (parse strings) → Constructor (typed params) → Instance
```

**Key takeaway:** Separate parsing (init method) from construction (constructor) for clean, type-safe directives!

---

## Need Help?

- See `include/Directives/ExampleDirective.hpp` for a working example
- Check `test_improvements.cpp` for usage examples
- Review `DirectiveFactory.hpp` for macro documentation
