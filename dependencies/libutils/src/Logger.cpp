#include "../include/Logger.hpp"
#include <fstream>
#include <cstdlib>

namespace Logger {

    // ============================================================================
    // Global configuration variables
    // ============================================================================
    
    // Default: INFO | WARNING | ERROR | CRITICAL (production-friendly)
    int g_logLevel = INFO | WARNING | ERROR | CRITICAL;
    
    // Show file and line information by default
    bool g_logShowLocation = true;
    
    // Use colored output by default (can be disabled for log files)
    bool g_logColored = true;
    
    // Log file path (empty = stdout/stderr only)
    std::string g_logFile = "";
    
    // Internal file stream for log file output
    static std::ofstream* g_logFileStream = NULL;

    // ============================================================================
    // Helper function implementations
    // ============================================================================

    /**
     * @brief Get ANSI color code for log level
     * @param level Log level enum value
     * @return ANSI color escape sequence
     */
    const char* getLevelColor(int level) {
        switch (level) {
            case TRACE:    return "\033[36m";      // Cyan
            case DEBUG:    return "\033[34m";      // Blue
            case INFO:     return "\033[32m";      // Green
            case WARNING:  return "\033[33m";      // Yellow
            case ERROR:    return "\033[31m";      // Red
            case CRITICAL: return "\033[31;1m";    // Bold Red
            default:       return "";
        }
    }

    /**
     * @brief Get error string from current errno
     * @return Error description from strerror(errno)
     */
    std::string getErrorString() {
        return std::string(strerror(errno));
    }

    /**
     * @brief Get error string from specific error number
     * @param errnum Error number
     * @return Error description from strerror(errnum)
     */
    std::string getErrorString(int errnum) {
        return std::string(strerror(errnum));
    }

    // ============================================================================
    // Configuration parsing functions
    // ============================================================================

    /**
     * @brief Parse log level string to bit flag
     * @param level String like "debug", "info", "error", etc.
     * @return Bit flag value or -1 if invalid
     * 
     * Supported values:
     * - "trace" -> TRACE (0x01)
     * - "debug" -> DEBUG (0x02)
     * - "info" -> INFO (0x04)
     * - "warning" -> WARNING (0x08)
     * - "error" -> ERROR (0x10)
     * - "critical" -> CRITICAL (0x20)
     * - "all" -> ALL (0x3F)
     * - "none" -> NONE (0x00)
     */
    int parseLevelString(const std::string& level) {
        if (level == "trace") return TRACE;
        if (level == "debug") return DEBUG;
        if (level == "info") return INFO;
        if (level == "warning") return WARNING;
        if (level == "error") return ERROR;
        if (level == "critical") return CRITICAL;
        if (level == "all") return ALL;
        if (level == "none") return NONE;
        return -1;  // Invalid level
    }

    /**
     * @brief Parse numeric log level (decimal or hex)
     * @param value String like "4", "16", "0x10", "0x3F", etc.
     * @return Bit mask value or -1 if invalid
     * 
     * Examples:
     * - "4" or "0x04" -> INFO
     * - "16" or "0x10" -> ERROR
     * - "20" or "0x14" -> INFO | ERROR
     * - "60" or "0x3C" -> INFO | WARNING | ERROR | CRITICAL
     * - "63" or "0x3F" -> ALL
     */
    int parseLevelNumber(const std::string& value) {
        if (value.empty()) {
            return -1;
        }
        
        char* end;
        long num;
        
        // Support both hexadecimal (0x10) and decimal (16)
        if (value.find("0x") == 0 || value.find("0X") == 0) {
            num = std::strtol(value.c_str(), &end, 16);
        } else {
            num = std::strtol(value.c_str(), &end, 10);
        }
        
        // Validate conversion
        if (*end != '\0' || num < 0 || num > 0x3F) {
            return -1;  // Invalid number
        }
        
        return static_cast<int>(num);
    }

    // ============================================================================
    // File management functions
    // ============================================================================

    /**
     * @brief Open log file for output
     * @param filepath Path to log file
     * @return true on success, false on failure
     */
    bool openLogFile(const std::string& filepath) {
        // Close existing file if open
        closeLogFile();
        
        if (filepath.empty()) {
            return false;
        }
        
        // Open new file in append mode
        g_logFileStream = new std::ofstream(filepath.c_str(), std::ios::app);
        if (!g_logFileStream || !g_logFileStream->is_open()) {
            delete g_logFileStream;
            g_logFileStream = NULL;
            return false;
        }
        
        g_logFile = filepath;
        return true;
    }

    /**
     * @brief Close log file
     */
    void closeLogFile() {
        if (g_logFileStream) {
            g_logFileStream->close();
            delete g_logFileStream;
            g_logFileStream = NULL;
        }
        g_logFile = "";
    }

    /**
     * @brief Get appropriate output stream for log level
     * @param level Log level
     * @return Reference to output stream (file, stdout, or stderr)
     */
    std::ostream& getLogStream(int level) {
        // If log file is open, use it
        if (g_logFileStream && g_logFileStream->is_open()) {
            return *g_logFileStream;
        }
        
        // Otherwise use stderr for errors, stdout for everything else
        if (level & (ERROR | CRITICAL)) {
            return std::cerr;
        }
        return std::cout;
    }

} // namespace Logger
