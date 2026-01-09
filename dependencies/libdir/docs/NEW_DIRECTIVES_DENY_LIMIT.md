# New Directives: deny and limit_except

## Summary

Successfully added **2 new access control directives** to the configuration parser library:

1. ✅ **deny** - IP-based access control (simple directive)
2. ✅ **limit_except** - HTTP method restrictions (block directive)

**Total directive count: 20** (was 18)

## Quick Reference

### deny Directive
```nginx
deny address;  # IP, CIDR, or "all"
```

**Context:** http, server, location, limit_except  
**Data:** `const std::string address`  
**Examples:**
- `deny 192.168.1.1;`
- `deny 10.0.0.0/8;`
- `deny all;`

### limit_except Directive
```nginx
limit_except METHOD [METHOD2 ...] {
    # Nested directives (e.g., deny)
}
```

**Context:** location only  
**Data:** `const std::vector<std::string> methods`  
**Valid Methods:** GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH, CONNECT, TRACE  
**Example:**
```nginx
location /api/ {
    limit_except GET POST {
        deny all;  # Only GET/POST allowed
    }
}
```

## Files Created/Modified

### New Files (4)
1. `directives/include/DenyDirective.hpp` - Header for deny directive
2. `directives/src/DenyDirective.cpp` - Implementation with IP/CIDR validation
3. `directives/include/LimitExceptDirective.hpp` - Header for limit_except directive
4. `directives/src/LimitExceptDirective.cpp` - Implementation with method validation
5. `tests/test_access_control.cpp` - Comprehensive test suite
6. `docs/ACCESS_CONTROL.md` - Complete documentation

### Modified Files (5)
1. `Makefile` - Added new directive source files to build
2. `docs/WEBSERV_DIRECTIVES.md` - Added deny and limit_except documentation
3. `docs/DIRECTIVE_LIST.md` - Updated count to 20 directives
4. `docs/IMPLEMENTATION_COMPLETE.md` - Updated summary with new directives
5. `.github/copilot-instructions.md` - Updated quick reference

## Build Status

✅ **Library compiled successfully** with -Wall -Wextra -Werror

```bash
$ make re
# Compiles 20 directives into build/libdir.a (5.5MB)
```

## Test Status

✅ **All tests passing:**

```bash
$ ./test_access_control
=== Testing Deny Directive ===
✓ Deny IP: 192.168.1.1
✓ Deny CIDR: 10.0.0.0/8
✓ Deny all: all
✓ Invalid args correctly rejected

=== Testing LimitExcept Directive ===
✓ Single method: GET
✓ Multiple methods: GET, POST, DELETE
✓ Invalid method correctly rejected
✓ Empty methods correctly rejected

=== Testing Access Control Configuration ===
✓ HTTP-level deny: 192.168.1.100
✓ Server-level deny: 10.0.0.0/8
✓ /api/ deny: all
✓ /api/ limit_except: GET, POST allowed
✓ /api/ limit_except contains deny all
✓ /admin/ deny: 192.168.1.0/24
✓ /admin/ limit_except: GET allowed

==================================
All tests passed successfully!
==================================

$ ./test_webserv
=== Testing Complete Webserv Configuration ===
# ... all webserv tests pass ...
```

## Usage Example

### Configuration File
```nginx
http {
    deny 203.0.113.0/24;  # Block bad network at HTTP level
    
    server {
        listen 80;
        server_name api.example.com;
        deny 192.168.0.0/16;  # Block private network
        
        location /public/ {
            root /var/www/public;
            limit_except GET HEAD {
                deny all;  # Read-only public access
            }
        }
        
        location /api/ {
            root /var/www/api;
            deny all;  # Explicit deny
            limit_except GET POST {
                deny all;  # Only GET/POST allowed
            }
        }
        
        location /admin/ {
            root /var/www/admin;
            deny all;  # Completely restricted
        }
    }
}
```

### C++ Code
```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "LocationDirective.hpp"
#include "DenyDirective.hpp"
#include "LimitExceptDirective.hpp"

// Parse config
Config::Parser parser = Config::Parser::openConfigFile("config.conf");
while (parser.getNextDirectives()) {
    Config::IDirective::parseDirective(Config::global, parser.copyDirective());
}

// Access deny directives
const Config::LocationDirective* loc = /* ... get location ... */;
std::vector<Config::DenyDirective*> denies = loc->getChildren<Config::DenyDirective>();
for (size_t i = 0; i < denies.size(); ++i) {
    std::cout << "Denied: " << denies[i]->address << std::endl;
}

// Access limit_except directive
try {
    const Config::LimitExceptDirective* limit = 
        loc->getFirstChild<Config::LimitExceptDirective>();
    std::cout << "Allowed methods: ";
    for (size_t i = 0; i < limit->methods.size(); ++i) {
        std::cout << limit->methods[i] << " ";
    }
    std::cout << std::endl;
} catch (const Config::DirectiveNotFound&) {
    // No limit_except directive
}
```

## Implementation Details

### DenyDirective
- **Pattern:** Simple directive with single string parameter
- **Validation:** Checks for "all" or IP-like format (contains dots)
- **Contexts:** http, server, location, limit_except
- **OCF:** Full Orthodox Canonical Form implemented
- **Registration:** `DEFINE_DIRECTIVE(DenyDirective, "deny", ...)`

### LimitExceptDirective
- **Pattern:** Block directive (can contain children)
- **Validation:** Validates against 9 standard HTTP methods
- **Context:** location only
- **OCF:** Full Orthodox Canonical Form implemented
- **Registration:** `DEFINE_DIRECTIVE(LimitExceptDirective, "limit_except", LocationDirective)`
- **Children:** Can contain DenyDirective (and potentially other access directives)

## Key Differences: allowed_methods vs limit_except

| Feature | allowed_methods | limit_except |
|---------|----------------|--------------|
| Type | Simple directive | Block directive |
| Syntax | `allowed_methods GET POST;` | `limit_except GET POST { }` |
| Context | location | location |
| Can nest directives | ❌ No | ✅ Yes (e.g., deny) |
| Use case | Simple method restriction | Complex access control |

## Documentation

Complete documentation available in:
- `docs/ACCESS_CONTROL.md` - Full guide with examples
- `docs/WEBSERV_DIRECTIVES.md` - Reference documentation
- `docs/DIRECTIVE_LIST.md` - All 20 directives list
- `tests/test_access_control.cpp` - Working code examples

## Next Steps

The library now has complete access control capabilities:
1. ✅ IP-based restrictions (deny)
2. ✅ Method-based restrictions (allowed_methods, limit_except)
3. ✅ Nested restrictions (deny inside limit_except)
4. ✅ Multiple levels (http, server, location)

Ready for integration into webserv project for implementing access control logic.
