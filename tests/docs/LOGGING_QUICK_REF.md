# 📋 Logging System Quick Reference

## Configuration

```nginx
http {
    # String configuration (recommended for readability)
    log_level info warning error critical;
    
    # Numeric configuration (power users)
    log_level 60;  # Same as above (4+8+16+32)
    
    # Control file:line display
    log_show_location on;   # Show [file.cpp:123]
    log_show_location off;  # Hide for cleaner logs
    
    # Control ANSI colors
    log_colored on;   # Use colors (default)
    log_colored off;  # Plain text
}
```

## Log Levels

| Level | Value | Decimal | Hex |
|-------|-------|---------|-----|
| trace | TRACE | 1 | 0x01 |
| debug | DEBUG | 2 | 0x02 |
| info | INFO | 4 | 0x04 |
| warning | WARNING | 8 | 0x08 |
| error | ERROR | 16 | 0x10 |
| critical | CRITICAL | 32 | 0x20 |
| all | ALL | 63 | 0x3F |
| none | NONE | 0 | 0x00 |

## Common Presets

```nginx
# Production (silent)
log_level 48;  # ERROR + CRITICAL

# Production (normal)
log_level 60;  # INFO + WARNING + ERROR + CRITICAL

# Development
log_level 63;  # ALL

# Debug specific issues
log_level 18;  # DEBUG + ERROR
```

## Code Usage

```cpp
#include "libutils/include/Logger.hpp"

// Basic logging
LOG_INFO("Server started");
LOG_DEBUG("Processing: " << data);
LOG_WARNING("Rate limit exceeded");
LOG_ERROR("File not found");
LOG_CRITICAL("Out of memory");

// With errno
LOG_ERROR_ERRNO("Failed to bind socket");
LOG_CRITICAL_ERRNO("malloc failed");

// With getaddrinfo
LOG_ERROR_GAI("DNS lookup failed", ret);
```

## Calculator

To combine levels:
```
INFO (4) + ERROR (16) + CRITICAL (32) = 52

Config: log_level 52;
```

## Testing

```bash
# All logging
./build/webserv conf/test_logging.conf

# Production mode
./build/webserv conf/test_production.conf

# Development mode
./build/webserv conf/test_development.conf
```

## Output Examples

**Full Debug:**
```
[2026-01-25 10:30:45] [DEBUG] [EventLoop.cpp:319] Added event for fd:5
```

**Production:**
```
[2026-01-25 10:30:45] [ERROR] Failed to bind socket: Address already in use
```

## Performance Notes

- ✅ Zero overhead when level disabled
- ✅ No string construction if not logging
- ✅ Lazy evaluation of expressions
- ✅ Dead code elimination by compiler
- ✅ Direct stream output (no copies)

## Documentation

See [LOGGING.md](LOGGING.md) for complete documentation.
