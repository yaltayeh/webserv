# 📊 Logging System Documentation

## Overview

High-performance logging system with bit mask log levels, zero-overhead macros, and flexible configuration.

---

## Features

✅ **Bit Mask Log Levels** - Enable multiple specific levels simultaneously  
✅ **Zero-Overhead Macros** - Lazy evaluation, no string construction when disabled  
✅ **errno Integration** - Automatic `strerror()` and `gai_strerror()` support  
✅ **Conditional File:Line** - Show/hide code locations for clean production logs  
✅ **Conditional Timestamp** - Show/hide timestamps for flexible output  
✅ **Colored Output** - ANSI colors for terminal visibility  
✅ **File Output** - Write logs to file for persistence  
✅ **Numeric Configuration** - Direct bit mask values for power users  
✅ **Performance** - Dead code elimination when log level disabled  

---

## Log Levels

### Bit Flags

| Level    | Decimal | Hex  | Description |
|----------|---------|------|-------------|
| TRACE    | 1       | 0x01 | Function entry/exit, detailed flow |
| DEBUG    | 2       | 0x02 | Debug info, variable states |
| INFO     | 4       | 0x04 | Informational messages |
| WARNING  | 8       | 0x08 | Warnings, recoverable issues |
| ERROR    | 16      | 0x10 | Errors, operation failures |
| CRITICAL | 32      | 0x20 | Critical errors before crash |
| ALL      | 63      | 0x3F | All log levels |
| NONE     | 0       | 0x00 | No logging |

### Common Combinations

| Use Case | Decimal | Hex  | Levels |
|----------|---------|------|--------|
| **Production (silent)** | 48 | 0x30 | ERROR + CRITICAL |
| **Production (normal)** | 60 | 0x3C | INFO + WARNING + ERROR + CRITICAL |
| **Development** | 63 | 0x3F | ALL |
| **Debug only** | 18 | 0x12 | DEBUG + ERROR |

---

## Configuration

### Basic Syntax

```nginx
http {
    # String-based (multiple levels)
    log_level debug info warning error critical;
    
    # String-based (single)
    log_level error;
    
    # Numeric (decimal)
    log_level 60;  # INFO | WARNING | ERROR | CRITICAL
    
    # Numeric (hexadecimal)
    log_level 0x3C;  # INFO | WARNING | ERROR | CRITICAL
    
    # All levels
    log_level all;
    
    # No logging
    log_level none;
    
    # Control file:line display
    log_show_location on;   # Show [file.cpp:123] (default)
    log_show_location off;  # Hide for cleaner logs
    
    # Control timestamp display
    log_show_timestamp on;  # Show [YYYY-MM-DD HH:MM:SS] (default)
    log_show_timestamp off; # Hide timestamps
    
    # Color output
    log_colored on;   # Use ANSI colors (default)
    log_colored off;  # Disable for log files
    
    # Log file output
    log_file /var/log/webserv.log;  # Write to file
    # If not specified, logs go to stdout/stderr
    
    server {
        # ...
    }
}
```

### Configuration Examples

#### Production (Minimal Logging)
```nginx
http {
    log_level 48;  # ERROR | CRITICAL only
    log_show_location off;
    log_colored on;
}
```

**Output:**
```
[2026-01-25 10:30:45] ERROR Failed to bind socket: Address already in use
[2026-01-25 10:30:46] CRITICAL Out of memory
```

#### Production (Normal)
```nginx
http {
    log_level info warning error critical;
    log_show_location off;
    log_colored on;
}
```

**Output:**
```
[2026-01-25 10:30:45] INFO Server listening on 0.0.0.0:8080
[2026-01-25 10:30:46] INFO Accepted connection from 127.0.0.1:54321
[2026-01-25 10:30:47] WARNING Client body size exceeds limit
```

#### Log File Output
```nginx
http {
    log_level info warning error critical;
    log_show_location off;
    log_show_timestamp off;  # No timestamps for cleaner file logs
    log_colored off;         # No ANSI codes in file
    log_file /var/log/webserv.log;
}
```

**Output in /var/log/webserv.log:**
```
[INFO] Server listening on 0.0.0.0:8080
[INFO] Accepted connection from 127.0.0.1:54321
[WARNING] Client body size exceeds limit
```

#### Development (Verbose with Locations)
```nginx
http {
    log_level all;
    log_show_location on;
    log_colored on;
}
```

**Output:**
```
[2026-01-25 10:30:45] DEBUG [EventLoop.cpp:319] Added event for fd:5
[2026-01-25 10:30:45] INFO [ListenSocket.cpp:172] Server listening on 0.0.0.0:8080
[2026-01-25 10:30:46] TRACE [StaticFileMiddleware.cpp:54] Handling URI: /index.html
[2026-01-25 10:30:46] DEBUG [ClientSocket.cpp:148] Read 145 bytes from client
```

---

## Usage in Code

### Standard Logging

```cpp
#include "libutils/include/Logger.hpp"

// Trace level (function flow)
LOG_TRACE("Entering handleRequest()");

// Debug level (detailed info)
LOG_DEBUG("Processing request: " << method << " " << uri);

// Info level (important events)
LOG_INFO("Server started on port " << port);

// Warning level (recoverable issues)
LOG_WARNING("Client body size " << size << " exceeds recommended limit");

// Error level (operation failures)
LOG_ERROR("Failed to open file: " << filename);

// Critical level (fatal errors)
LOG_CRITICAL("Out of memory, cannot continue");
```

### errno Integration

```cpp
// Automatic errno handling with strerror()
if (bind(sockfd, ...) == -1) {
    LOG_ERROR_ERRNO("Failed to bind socket");
    // Output: [ERROR] [Socket.cpp:45] Failed to bind socket: Address already in use
}

// Critical errors with errno
if (malloc(...) == NULL) {
    LOG_CRITICAL_ERRNO("Memory allocation failed");
    exit(1);
}
```

### getaddrinfo() Errors

```cpp
int ret = getaddrinfo(...);
if (ret != 0) {
    LOG_ERROR_GAI("getaddrinfo failed", ret);
    // Output: [ERROR] getaddrinfo failed: Name or service not known
}
```

---

## Performance Characteristics

### Zero-Overhead Design

**When log level is disabled:**
```cpp
LOG_DEBUG("Expensive operation: " << expensiveFunction());

// Compiles to:
if (false) {  // Compiler knows DEBUG is disabled
    // This entire block is eliminated by optimizer
}

// Result: ZERO assembly instructions!
```

### Lazy Evaluation

```cpp
// Function NEVER called when DEBUG disabled
LOG_DEBUG("Vector: " << vectorToString(vec));

// With function-based logging: vectorToString() ALWAYS runs
// With macro-based logging: vectorToString() ONLY runs if DEBUG enabled
```

### Binary Size Impact

- Pure macro implementation → smaller binary
- No function call overhead
- Dead code elimination by compiler
- Inline helpers for minimal footprint

---

## Color Codes

When `log_colored on`:

- **TRACE**: Cyan
- **DEBUG**: Blue
- **INFO**: Green
- **WARNING**: Yellow
- **ERROR**: Red
- **CRITICAL**: Bold Red

---

## Best Practices

### Development
```nginx
log_level all;
log_show_location on;
log_colored on;
```

### Testing
```nginx
log_level debug info warning error critical;
log_show_location on;
log_colored on;
```

### Staging
```nginx
log_level info warning error critical;
log_show_location off;
log_colored on;
```

### Production
```nginx
log_level error critical;
log_show_location off;
log_show_timestamp on;   # Keep timestamps in production
log_colored off;
log_file /var/log/webserv.log;  # File output for persistence
```

---

## Numeric Log Level Calculator

To calculate custom bit masks:

```
TRACE    = 1
DEBUG    = 2
INFO     = 4
WARNING  = 8
ERROR    = 16
CRITICAL = 32

Example: INFO + ERROR + CRITICAL
= 4 + 16 + 32
= 52 (decimal) = 0x34 (hex)

Config: log_level 52;
```

---

## Implementation Details

### File Structure
```
dependencies/libutils/
├── include/
│   └── Logger.hpp       # Macro definitions and declarations
└── src/
    └── Logger.cpp       # Helper function implementations

src/directives/
├── LogLevelDirective.cpp        # Parse log_level directive
├── LogShowLocationDirective.cpp # Parse log_show_location directive
└── LogColoredDirective.cpp      # Parse log_colored directive
```

### Global Variables
```cpp
extern int g_logLevel;          // Bit mask (default: INFO|WARNING|ERROR|CRITICAL)
extern bool g_logShowLocation;  // Show file:line (default: true)
extern bool g_logColored;       // Use colors (default: true)
extern bool g_logShowTimestamp; // Show timestamp (default: true)
extern std::string g_logFile;   // Log file path (default: empty/stdout/stderr)
```

### Macro Architecture
```cpp
#define LOG_DEBUG(msg) \
    do { \
        if (Logger::g_logLevel & Logger::DEBUG) { \
            std::cout << Logger::getTimestamp() << " " \
                      << Logger::getLevelColor(Logger::DEBUG) \
                      << "[DEBUG]" << "\033[0m " \
                      << (Logger::g_logShowLocation ? \
                          "[" << __FILE__ << ":" << __LINE__ << "] " : "") \
                      << msg << std::endl; \
        } \
    } while(0)
```

---

## Testing

Test different configurations:

```bash
# All levels
./build/webserv conf/test_logging.conf

# Production mode
./build/webserv conf/test_production.conf

# Development mode
./build/webserv conf/test_development.conf

# Numeric configuration
./build/webserv conf/test_numeric.conf
```

---

## Troubleshooting

**Q: No logs appearing?**  
A: Check `log_level` in config. Default is `INFO|WARNING|ERROR|CRITICAL`.

**Q: Too many logs?**  
A: Use `log_level error critical;` for production.

**Q: Need to debug specific issue?**  
A: Enable only needed levels: `log_level debug error;`

**Q: File/line info cluttering logs?**  
A: Disable with `log_show_location off;`

**Q: Colors not showing?**  
A: Ensure `log_colored on;` and terminal supports ANSI.

---

## Future Enhancements

- [x] ~~Log file output~~ ✅ **Implemented!**
- [x] ~~Timestamp control~~ ✅ **Implemented!**
- [ ] Log rotation
- [ ] Per-server log levels
- [ ] Per-location log levels
- [ ] Request ID tracking
- [ ] Async logging for high performance
- [ ] JSON log format option

---

## Summary

The logging system provides **maximum flexibility** with **zero performance overhead** when disabled. Use bit masks to enable exactly the log levels you need, from silent production mode to full debug tracing during development.

**Key Benefits:**
- 🚀 Fast: Zero overhead when disabled
- 🎯 Precise: Bit mask combinations
- 🔧 Flexible: String or numeric config
- 📍 Optional: File/line information
- 🎨 Colorful: ANSI terminal colors
- 💪 Robust: errno/strerror integration
