#include "../include/StringUtils.hpp"
#include <cctype>
#include <sstream>
#include <cstdlib>
#include <climits>

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

std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // Check if next two characters are valid hex digits
            char hex1 = str[i + 1];
            char hex2 = str[i + 2];
            
            if (isxdigit(static_cast<unsigned char>(hex1)) && 
                isxdigit(static_cast<unsigned char>(hex2))) {
                // Convert hex to decimal
                int value = 0;
                
                // First hex digit
                if (hex1 >= '0' && hex1 <= '9') {
                    value = (hex1 - '0') * 16;
                } else if (hex1 >= 'a' && hex1 <= 'f') {
                    value = (hex1 - 'a' + 10) * 16;
                } else if (hex1 >= 'A' && hex1 <= 'F') {
                    value = (hex1 - 'A' + 10) * 16;
                }
                
                // Second hex digit
                if (hex2 >= '0' && hex2 <= '9') {
                    value += (hex2 - '0');
                } else if (hex2 >= 'a' && hex2 <= 'f') {
                    value += (hex2 - 'a' + 10);
                } else if (hex2 >= 'A' && hex2 <= 'F') {
                    value += (hex2 - 'A' + 10);
                }
                
                result += static_cast<char>(value);
                i += 2; // Skip the two hex digits
            } else {
                // Not valid hex, keep the % as-is
                result += str[i];
            }
        } else if (str[i] == '+') {
            // Convert + to space (common in query strings)
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

// ============================================================================
// Path Utilities
// ============================================================================

std::string getDirectory(const std::string& path) {
    size_t lastSlash = path.rfind('/');
    if (lastSlash != std::string::npos) {
        // Special case: file in root directory (e.g., "/file.txt")
        if (lastSlash == 0) {
            return "/";
        }
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
    // Check for embedded null bytes
    // Note: std::string can contain null bytes if constructed with explicit length
    if (path.find('\0') != std::string::npos) {
        return true;
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

std::string sizeToString(size_t value) {
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
    
    // Check for overflow before casting to int
    if (result > INT_MAX || result < INT_MIN) {
        return defaultValue;
    }
    
    return static_cast<int>(result);
}

} // namespace utils
