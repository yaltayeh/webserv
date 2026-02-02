#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <sys/time.h>

/**
 * @brief High-performance logging system with bit mask log levels
 * 
 * Features:
 * - Bit mask log levels for flexible combinations
 * - Zero-overhead macros with lazy evaluation
 * - Conditional file/line information
 * - errno/strerror integration
 * - Colored terminal output
 * - Numeric and string log level configuration
 */
namespace Logger {
    
    // Log level bit flags
    enum LogLevel {
        TRACE    = 1 << 0,  // 0x01 = 1    - Function entry/exit, detailed flow
        DEBUG    = 1 << 1,  // 0x02 = 2    - Debug information, variable states
        INFO     = 1 << 2,  // 0x04 = 4    - Informational messages
        WARNING  = 1 << 3,  // 0x08 = 8    - Warnings, recoverable issues
        ERROR    = 1 << 4,  // 0x10 = 16   - Errors, operation failures
        CRITICAL = 1 << 5,  // 0x20 = 32   - Critical errors before crash
        ALL      = 0x3F,    // 63          - All log levels
        NONE     = 0        // 0           - No logging
    };

    // Global configuration variables
    extern int g_logLevel;          // Bit mask for enabled log levels
    extern bool g_logShowLocation;  // Show file:line information
    extern bool g_logColored;       // Use ANSI color codes
    extern std::string g_logFile;   // Log file path (empty = stdout/stderr only)

    // Lightweight inline helper functions
    inline const char* getFileName(const char* path) {
        if (!path) return "";
        const char* slash = strrchr(path, '/');
        return slash ? slash + 1 : path;
    }
    
    inline std::string toString(int n) {
        std::ostringstream oss;
        oss << n;
        return oss.str();
    }

    // Helper functions implemented in Logger.cpp
    const char* getLevelColor(int level);
    std::string getErrorString();
    std::string getErrorString(int errnum);
    
    // File management
    bool openLogFile(const std::string& filepath);
    void closeLogFile();
    std::ostream& getLogStream(int level);  // Returns file or stdout/stderr
    
    // Configuration parsing
    int parseLevelString(const std::string& level);
    int parseLevelNumber(const std::string& value);

} // namespace Logger

// ============================================================================
// Internal implementation macro - all logging goes through this
// ============================================================================
#define LOG_IMPL(level, level_name, default_stream, msg) \
    do { \
        if (Logger::g_logLevel & (level)) { \
            std::ostream& stream = Logger::getLogStream(level); \
            if (Logger::g_logColored) { \
                stream << Logger::getLevelColor(level); \
            } \
            stream << "[" << level_name << "]"; \
            if (Logger::g_logColored) { \
                stream << "\033[0m"; \
            } \
            stream << " "; \
            if (Logger::g_logShowLocation) { \
                stream << "[" << Logger::getFileName(__FILE__) << ":" << __LINE__ << "] "; \
            } \
            stream << msg << std::endl; \
        } \
    } while(0)

// ============================================================================
// Public logging macros
// ============================================================================

/**
 * @brief Log trace-level message (function entry/exit, detailed flow)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_TRACE("Entering function with param=" << param);
 */
#define LOG_TRACE(msg) LOG_IMPL(Logger::TRACE, "TRACE", std::cout, msg)

/**
 * @brief Log debug-level message (debug info, variable states)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_DEBUG("Processing request: " << method << " " << uri);
 */
#define LOG_DEBUG(msg) LOG_IMPL(Logger::DEBUG, "DEBUG", std::cout, msg)

/**
 * @brief Log info-level message (informational)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_INFO("Server started on port " << port);
 */
#define LOG_INFO(msg) LOG_IMPL(Logger::INFO, "INFO", std::cout, msg)

/**
 * @brief Log warning-level message (warnings, recoverable issues)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_WARNING("Client body size exceeds limit");
 */
#define LOG_WARNING(msg) LOG_IMPL(Logger::WARNING, "WARNING", std::cout, msg)

/**
 * @brief Log error-level message (operation failures)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_ERROR("Failed to open file: " << filename);
 */
#define LOG_ERROR(msg) LOG_IMPL(Logger::ERROR, "ERROR", std::cerr, msg)

/**
 * @brief Log critical-level message (fatal errors before crash)
 * @param msg Message to log (can use << operators)
 * 
 * Example: LOG_CRITICAL("Out of memory, cannot continue");
 */
#define LOG_CRITICAL(msg) LOG_IMPL(Logger::CRITICAL, "CRITICAL", std::cerr, msg)

// ============================================================================
// Errno-integrated logging macros
// ============================================================================

/**
 * @brief Log error with errno automatically appended
 * @param msg Base message (errno and strerror added automatically)
 * 
 * Example: LOG_ERROR_ERRNO("Failed to bind socket");
 * Output: [ERROR] Failed to bind socket: Address already in use
 */
#define LOG_ERROR_ERRNO(msg) \
    do { \
        if (Logger::g_logLevel & Logger::ERROR) { \
            int saved_errno = errno; \
            std::ostream& stream = Logger::getLogStream(Logger::ERROR); \
            if (Logger::g_logColored) stream << "\033[31m"; \
            stream << "[ERROR]"; \
            if (Logger::g_logColored) stream << "\033[0m"; \
            stream << " "; \
            if (Logger::g_logShowLocation) { \
                stream << "[" << Logger::getFileName(__FILE__) << ":" << __LINE__ << "] "; \
            } \
            stream << msg << ": " << strerror(saved_errno) << std::endl; \
        } \
    } while(0)

/**
 * @brief Log critical error with errno automatically appended
 * @param msg Base message (errno and strerror added automatically)
 * 
 * Example: LOG_CRITICAL_ERRNO("Failed to allocate memory");
 */
#define LOG_CRITICAL_ERRNO(msg) \
    do { \
        if (Logger::g_logLevel & Logger::CRITICAL) { \
            int saved_errno = errno; \
            std::ostream& stream = Logger::getLogStream(Logger::CRITICAL); \
            if (Logger::g_logColored) stream << "\033[31;1m"; \
            stream << "[CRITICAL]"; \
            if (Logger::g_logColored) stream << "\033[0m"; \
            stream << " "; \
            if (Logger::g_logShowLocation) { \
                stream << "[" << Logger::getFileName(__FILE__) << ":" << __LINE__ << "] "; \
            } \
            stream << msg << ": " << strerror(saved_errno) << std::endl; \
        } \
    } while(0)

/**
 * @brief Log error from getaddrinfo with gai_strerror
 * @param msg Base message
 * @param errcode Error code returned from getaddrinfo
 * 
 * Example: 
 * int ret = getaddrinfo(...);
 * if (ret != 0) LOG_ERROR_GAI("getaddrinfo failed", ret);
 */
#define LOG_ERROR_GAI(msg, errcode) \
    do { \
        if (Logger::g_logLevel & Logger::ERROR) { \
            std::ostream& stream = Logger::getLogStream(Logger::ERROR); \
            if (Logger::g_logColored) stream << "\033[31m"; \
            stream << "[ERROR]"; \
            if (Logger::g_logColored) stream << "\033[0m"; \
            stream << " "; \
            if (Logger::g_logShowLocation) { \
                stream << "[" << Logger::getFileName(__FILE__) << ":" << __LINE__ << "] "; \
            } \
            stream << msg << ": " << gai_strerror(errcode) << std::endl; \
        } \
    } while(0)
