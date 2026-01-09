# Logging System Documentation

## Overview

The library includes a lightweight, C++98-compatible logging system with compile-time overhead elimination through macro-based conditional compilation.

## Log Levels

```cpp
Config::LOG_LEVEL_NONE   = -1  // Disable all logging
Config::LOG_LEVEL_ERROR  = 0   // Errors only
Config::LOG_LEVEL_WARN   = 1   // Warnings and errors
Config::LOG_LEVEL_INFO   = 2   // Info, warnings, and errors
Config::LOG_LEVEL_DEBUG  = 3   // All messages including debug
```

## Global Log Level Control

```cpp
#include "Logger.hpp"

// Set at program startup
Config::g_log_level = Config::LOG_LEVEL_DEBUG;  // Development
Config::g_log_level = Config::LOG_LEVEL_WARN;   // Production (default)
Config::g_log_level = Config::LOG_LEVEL_NONE;   // Performance critical
```

## Logging Macros

### LOGE(msg) - Error Messages
Always displayed unless `LOG_LEVEL_NONE`. Use for critical errors.

```cpp
LOGE("Failed to open file: " << filename);
LOGE("Parse error at line " << line_num);
```

### LOGW(msg) - Warning Messages
Displayed at `LOG_LEVEL_WARN` and above. Use for recoverable issues.

```cpp
LOGW("Directive '" << name << "' not found, using default");
LOGW("Deprecated syntax detected");
```

### LOGI(msg) - Info Messages
Displayed at `LOG_LEVEL_INFO` and above. Use for important events.

```cpp
LOGI("Loaded configuration from " << filepath);
LOGI("Found " << servers.size() << " server blocks");
```

### LOGD(msg) - Debug Messages
Displayed only at `LOG_LEVEL_DEBUG`. Use for development/troubleshooting.

```cpp
LOGD("Parsing directive: " << directive_name);
LOGD("Created " << typeid(*directive).name() << " at " << directive);
```

## Usage Examples

### Development Mode

```cpp
#include "Config.hpp"
#include "Parser.hpp"
#include "Logger.hpp"

int main() {
    // Enable verbose logging for debugging
    Config::g_log_level = Config::LOG_LEVEL_DEBUG;
    
    LOGI("Starting configuration parser");
    
    try {
        Config::Parser parser = Config::Parser::openConfigFile("config.conf");
        
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            LOGD("Found directive: " << directive);
            Config::IDirective::parseDirective(Config::global, directive);
        }
        
        LOGI("Configuration loaded successfully");
        
    } catch (const Config::ConfigException& e) {
        LOGE("Configuration error: " << e.what());
        return 1;
    }
    
    return 0;
}
```

### Production Mode

```cpp
int main() {
    // Minimal logging for production
    Config::g_log_level = Config::LOG_LEVEL_ERROR;
    
    try {
        Config::Parser parser = Config::Parser::openConfigFile("webserv.conf");
        while (parser.getNextDirectives()) {
            Config::IDirective::parseDirective(Config::global, parser.copyDirective());
        }
    } catch (const Config::ConfigException& e) {
        LOGE("Fatal error: " << e.what());
        return 1;
    }
    
    return 0;
}
```

### Performance Critical Mode

```cpp
int main() {
    // Disable ALL logging (zero overhead)
    Config::g_log_level = Config::LOG_LEVEL_NONE;
    
    // All LOGE/LOGW/LOGI/LOGD calls become no-ops
    // Compiler optimizes them away completely
}
```

## Implementation Details

### Zero-Overhead Design

The macros check `g_log_level` at runtime, but when compiled with optimizations (`-O2`, `-O3`), the compiler can eliminate dead code:

```cpp
#define LOGI(msg) \
    do { \
        if (Config::g_log_level >= Config::LOG_LEVEL_INFO) { \
            std::cout << "[INFO] " << msg << std::endl; \
        } \
    } while (0)
```

When `g_log_level = LOG_LEVEL_NONE`, the `if` condition is always false, and optimizers remove the entire block.

### C++98 Compatibility

- Uses `do-while(0)` macro pattern for statement safety
- Stream-based output (works with any type supporting `operator<<`)
- No variadic templates or C++11 features
- Compatible with strict C++98 compilation (`-std=c++98`)

### Thread Safety

**WARNING:** The current implementation is **NOT thread-safe**. If using in multi-threaded contexts:

1. Set `g_log_level` once at program startup before spawning threads
2. Don't modify `g_log_level` after threads are created
3. For thread-safe logging, add mutex protection to `std::cout`/`std::cerr` access

## Best Practices

1. **Set log level early**: In `main()` before any parsing
2. **Use appropriate levels**: 
   - `LOGE` for unrecoverable errors
   - `LOGW` for warnings that need attention
   - `LOGI` for high-level operation flow
   - `LOGD` for detailed debugging only
3. **Stream syntax**: Use `<<` for composing messages with variables
4. **Production builds**: Set to `LOG_LEVEL_ERROR` or `LOG_LEVEL_NONE`
5. **Development**: Use `LOG_LEVEL_DEBUG` for troubleshooting

## Integration with Existing Code

The logging system is completely optional. Existing code continues to work without any modifications. To add logging to existing directives:

```cpp
// In directive init methods
RootDirective* RootDirective::initRootDirective(const std::vector<std::string>& args) {
    LOGD("Initializing root directive with " << args.size() << " arguments");
    
    if (args.size() != 2) {
        LOGE("root directive: expected 1 argument, got " << (args.size() - 1));
        throw ArgumentException("root", "takes exactly one argument");
    }
    
    LOGD("Created root directive with path: " << args[1]);
    return new RootDirective(args[1]);
}
```

## Files

- `include/Logger.hpp` - Macro definitions and enum declarations
- `src/Logger.cpp` - Global variable definition (default: `LOG_LEVEL_WARN`)
- `tests/test_logging.cpp` - Example usage demonstration
