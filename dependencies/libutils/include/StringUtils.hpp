#pragma once

#include <string>

/**
 * @brief General utilities library
 * 
 * Provides reusable string manipulation, path handling, and validation utilities.
 * Follows DRY principle - these utilities are shared across all components.
 */
namespace utils {

// ============================================================================
// String Utilities
// ============================================================================

/**
 * @brief Trim whitespace from both ends of a string
 */
std::string trim(const std::string& str);

/**
 * @brief Trim whitespace from the left side of a string
 */
std::string trimLeft(const std::string& str);

/**
 * @brief Trim whitespace from the right side of a string
 */
std::string trimRight(const std::string& str);

/**
 * @brief Convert string to uppercase
 */
std::string toUpper(const std::string& str);

/**
 * @brief Convert string to lowercase
 */
std::string toLower(const std::string& str);

/**
 * @brief Decode URL-encoded string (e.g., %20 to space)
 * @param str URL-encoded string
 * @return Decoded string
 */
std::string urlDecode(const std::string& str);

// ============================================================================
// Path Utilities
// ============================================================================

/**
 * @brief Get the directory portion of a path
 * @param path Full file path
 * @return Directory path, or "." if no directory
 */
std::string getDirectory(const std::string& path);

/**
 * @brief Get the filename from a path
 * @param path Full file path
 * @return Filename portion
 */
std::string getFilename(const std::string& path);

/**
 * @brief Get the file extension from a path
 * @param path Full file path
 * @return Extension including dot (e.g., ".txt"), or empty string
 */
std::string getExtension(const std::string& path);

/**
 * @brief Check if a path has a specific extension
 * @param path File path
 * @param extension Extension to check (e.g., ".py")
 * @return true if path ends with the extension
 */
bool hasExtension(const std::string& path, const std::string& extension);

// ============================================================================
// Validation Utilities
// ============================================================================

/**
 * @brief Check if a port number is valid (1-65535)
 */
bool isValidPort(int port);

/**
 * @brief Check if a path contains path traversal sequences (..)
 */
bool containsPathTraversal(const std::string& path);

/**
 * @brief Sanitize a string for use as an environment variable value
 * Removes null bytes, newlines, and carriage returns
 */
std::string sanitizeForEnv(const std::string& value);

/**
 * @brief Check if a string contains only printable ASCII characters
 */
bool isPrintableAscii(const std::string& str);

// ============================================================================
// Conversion Utilities
// ============================================================================

/**
 * @brief Convert an integer to a string
 */
std::string intToString(int value);

/**
 * @brief Convert a size_t to a string
 */
std::string sizeToString(size_t value);

/**
 * @brief Convert a string to an integer
 * @param str String to convert
 * @param defaultValue Value to return if conversion fails
 * @return Converted integer or defaultValue
 */
int stringToInt(const std::string& str, int defaultValue = 0);

} // namespace utils

