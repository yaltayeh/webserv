#include "../include/StringUtils.hpp"
#include <cctype>
#include <sstream>
#include <cstdlib>

namespace utils {

// ============================================================================
// String Utilities
// ============================================================================

std::string trim(const std::string& str) {
    return trimLeft(trimRight(str));
}

std::string trimLeft(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    return str.substr(start);
}

std::string trimRight(const std::string& str) {
    size_t end = str.find_last_not_of(" \t\r\n");
    if (end == std::string::npos) {
        return "";
    }
    return str.substr(0, end + 1);
}

std::string toUpper(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    for (size_t i = 0; i < str.length(); ++i) {
        result += static_cast<char>(toupper(static_cast<unsigned char>(str[i])));
    }
    return result;
}

std::string toLower(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    for (size_t i = 0; i < str.length(); ++i) {
        result += static_cast<char>(tolower(static_cast<unsigned char>(str[i])));
    }
    return result;
}

// ============================================================================
// Path Utilities
// ============================================================================

std::string getDirectory(const std::string& path) {
    size_t lastSlash = path.rfind('/');
    if (lastSlash != std::string::npos) {
        return path.substr(0, lastSlash);
    }
    return ".";
}

std::string getFilename(const std::string& path) {
    size_t lastSlash = path.rfind('/');
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

std::string getExtension(const std::string& path) {
    size_t dotPos = path.rfind('.');
    size_t slashPos = path.rfind('/');
    
    // Ensure dot is after last slash (not in directory name)
    if (dotPos != std::string::npos && 
        (slashPos == std::string::npos || dotPos > slashPos)) {
        return path.substr(dotPos);
    }
    return "";
}

bool hasExtension(const std::string& path, const std::string& extension) {
    if (path.length() < extension.length() || extension.empty()) {
        return false;
    }
    return path.compare(path.length() - extension.length(), 
                        extension.length(), extension) == 0;
}

// ============================================================================
// Validation Utilities
// ============================================================================

bool isValidPort(int port) {
    return port > 0 && port <= 65535;
}

bool containsPathTraversal(const std::string& path) {
    // Check for path traversal patterns: "../" or "/.." 
    // Also check for ".." at the start or end of path
    if (path.find("../") != std::string::npos) {
        return true;
    }
    if (path.find("/..") != std::string::npos) {
        return true;
    }
    // Check if path starts with ".."
    if (path.length() >= 2 && path[0] == '.' && path[1] == '.') {
        // Allow if followed by something other than '/' or end of string
        if (path.length() == 2 || path[2] == '/') {
            return true;
        }
    }
    // Check for embedded null bytes by iterating through the string
    for (size_t i = 0; i < path.length(); ++i) {
        if (path[i] == '\0') {
            return true;
        }
    }
    return false;
}

std::string sanitizeForEnv(const std::string& value) {
    std::string result;
    result.reserve(value.length());
    for (size_t i = 0; i < value.length(); ++i) {
        char c = value[i];
        // Remove null bytes, newlines, and carriage returns
        if (c != '\0' && c != '\n' && c != '\r') {
            result += c;
        }
    }
    return result;
}

bool isPrintableAscii(const std::string& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c < 32 || c > 126) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// Conversion Utilities
// ============================================================================

std::string intToString(int value) {
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

int stringToInt(const std::string& str, int defaultValue) {
    if (str.empty()) {
        return defaultValue;
    }
    
    char* endPtr;
    long result = strtol(str.c_str(), &endPtr, 10);
    
    // Check if conversion was successful
    if (*endPtr != '\0') {
        return defaultValue;
    }
    
    return static_cast<int>(result);
}

} // namespace utils
