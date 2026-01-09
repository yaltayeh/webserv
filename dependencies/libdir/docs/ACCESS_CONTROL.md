# Access Control Directives

## Overview

The library now includes two access control directives for managing request permissions:

1. **deny** - IP-based access control
2. **limit_except** - HTTP method restrictions

## deny Directive

### Syntax
```nginx
deny address;
```

### Context
- http
- server
- location
- limit_except

### Description
Denies access from specified IP address, CIDR range, or all addresses.

### Data Members
```cpp
const std::string address;  // IP, CIDR notation, or "all"
```

### Valid Values
- **IP Address**: `192.168.1.1`
- **CIDR Notation**: `10.0.0.0/8`, `192.168.1.0/24`
- **All**: `all` (deny all addresses)

### Examples
```nginx
http {
    deny 192.168.1.100;  # Block specific IP at HTTP level
    
    server {
        listen 80;
        deny 10.0.0.0/8;  # Block entire private network
        
        location /public/ {
            # No deny - publicly accessible
        }
        
        location /admin/ {
            deny all;  # Block all access
        }
    }
}
```

### Usage in Code
```cpp
try {
    const Config::DenyDirective* deny = location->getFirstChild<Config::DenyDirective>();
    std::cout << "Denied address: " << deny->address << std::endl;
} catch (const Config::DirectiveNotFound&) {
    // No deny directive - access allowed
}

// Or get all deny directives (multiple allowed)
std::vector<Config::DenyDirective*> denies = location->getChildren<Config::DenyDirective>();
for (size_t i = 0; i < denies.size(); ++i) {
    std::cout << "Deny: " << denies[i]->address << std::endl;
}
```

## limit_except Directive

### Syntax
```nginx
limit_except method [method2 ...] {
    # Additional directives (e.g., deny)
}
```

### Context
- location (only)

### Description
Block directive that specifies which HTTP methods are **ALLOWED**. All other methods are **DENIED**.

This is the inverse of `allowed_methods`:
- `allowed_methods GET POST` - Simple directive, lists allowed methods
- `limit_except GET POST { }` - Block directive, can contain other directives like `deny`

### Data Members
```cpp
const std::vector<std::string> methods;  // List of allowed methods
```

### Valid Methods
- GET
- POST
- PUT
- DELETE
- HEAD
- OPTIONS
- PATCH
- CONNECT
- TRACE

### Examples

#### Basic Usage
```nginx
location /api/ {
    limit_except GET POST {
        # GET and POST are allowed
        # All other methods (PUT, DELETE, etc.) are denied
    }
}
```

#### Combined with deny
```nginx
location /api/ {
    # Allow GET and POST from anywhere
    # For all other methods, deny all access
    limit_except GET POST {
        deny all;
    }
}
```

#### Read-Only API
```nginx
location /readonly-api/ {
    limit_except GET HEAD {
        # Only GET and HEAD allowed
        # POST, PUT, DELETE, etc. are denied
    }
}
```

### Usage in Code
```cpp
try {
    const Config::LimitExceptDirective* limit = location->getFirstChild<Config::LimitExceptDirective>();
    
    // Check allowed methods
    std::cout << "Allowed methods: ";
    for (size_t i = 0; i < limit->methods.size(); ++i) {
        std::cout << limit->methods[i] << " ";
    }
    std::cout << std::endl;
    
    // Check for deny directives inside limit_except
    std::vector<Config::DenyDirective*> denies = limit->getChildren<Config::DenyDirective>();
    for (size_t i = 0; i < denies.size(); ++i) {
        std::cout << "Additional deny: " << denies[i]->address << std::endl;
    }
    
} catch (const Config::DirectiveNotFound&) {
    // No limit_except - all methods allowed (or use allowed_methods)
}
```

## Comparison: allowed_methods vs limit_except

### allowed_methods (Simple Directive)
```nginx
location /api/ {
    allowed_methods GET POST;  # Only these methods work
}
```
- Single line
- Cannot contain other directives
- Allowed in: location

### limit_except (Block Directive)
```nginx
location /api/ {
    limit_except GET POST {    # These methods are ALLOWED
        deny all;              # Can contain additional restrictions
    }
}
```
- Block with curly braces
- Can contain `deny` and other directives
- More complex access control
- Allowed in: location

## Complete Example

```nginx
http {
    # Block known attacker IP at HTTP level
    deny 203.0.113.0/24;
    
    server {
        listen 80;
        server_name api.example.com;
        
        # Block private network at server level
        deny 192.168.0.0/16;
        
        # Public read-only endpoint
        location /public/data/ {
            root /var/www/data;
            limit_except GET HEAD {
                deny all;  # Only allow GET/HEAD
            }
        }
        
        # Protected API with method restrictions
        location /api/v1/ {
            root /var/www/api;
            
            # Allow specific trusted IP
            # For restricted methods, use limit_except
            limit_except GET POST {
                # PUT, DELETE, etc. only from admin network
                deny all;
                # In real implementation, you'd have 'allow 10.0.1.0/24;' first
            }
        }
        
        # Admin area - completely restricted
        location /admin/ {
            root /var/www/admin;
            deny all;  # Block everyone
        }
    }
}
```

## Implementation Details

### deny Directive
- **File**: `directives/src/DenyDirective.cpp`
- **Header**: `directives/include/DenyDirective.hpp`
- **Registration**: `DEFINE_DIRECTIVE(DenyDirective, "deny", HttpDirective, ServerDirective, LocationDirective, LimitExceptDirective)`
- **Validation**: Basic IP format check (looks for dots or "all")
- **Member**: `const std::string address`

### limit_except Directive
- **File**: `directives/src/LimitExceptDirective.cpp`
- **Header**: `directives/include/LimitExceptDirective.hpp`
- **Registration**: `DEFINE_DIRECTIVE(LimitExceptDirective, "limit_except", LocationDirective)`
- **Validation**: Validates against 9 standard HTTP methods
- **Member**: `const std::vector<std::string> methods`
- **Block Directive**: Can contain child directives (like `deny`)

## Testing

See `tests/test_access_control.cpp` for comprehensive test coverage:

```bash
# Compile test
c++ -Wall -Wextra -Werror -std=c++98 -I./include -I./directives/include \
    tests/test_access_control.cpp build/libdir.a -o test_access_control

# Run test
./test_access_control
```

The test covers:
- ✅ deny with IP addresses
- ✅ deny with CIDR notation
- ✅ deny all
- ✅ limit_except with single method
- ✅ limit_except with multiple methods
- ✅ deny inside limit_except block
- ✅ Multiple levels of deny (http, server, location)
- ✅ Error handling for invalid methods
- ✅ Error handling for invalid arguments
