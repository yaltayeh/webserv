/**
 * RouteUtils.hpp - Shared utilities for middleware functions
 *
 * WHAT THIS PROVIDES:
 * Common helper functions used by multiple middleware:
 * - Error response generation
 * - Path resolution (root/alias handling)
 * - File system operations
 * - MIME type detection
 * - Index file lookup
 *
 * DESIGN:
 * - All functions are static/namespace-level (stateless)
 * - Returns heap-allocated Response* for error pages
 * - Reusable across all middleware
 */
#pragma once

#include "Config/LocationConfig.hpp"
#include "libhttp/include/Response.hpp"
#include <string>

namespace routing
{
namespace utils
{

/**
 * generateErrorResponse() - Creates error response with custom error page
 *
 * PARAMETERS:
 * - status_code: HTTP status code (400, 403, 404, 500, etc.)
 * - location: Configuration that may contain custom error_page mappings
 *
 * RETURNS:
 * - Heap-allocated Response* (caller must delete)
 *
 * BEHAVIOR:
 * - Checks for custom error_page configuration first
 * - Falls back to default HTML error page if custom page not found
 */
libhttp::Response *generateErrorResponse(int status_code,
                                         const LocationConfig &location);

/**
 * resolvePath() - Resolves URI to filesystem path using root or alias
 *
 * PARAMETERS:
 * - uri: Request URI (e.g., "/images/cat.jpg")
 * - location_path: Location block path (e.g., "/images")
 * - location: Configuration with root/alias settings
 *
 * RETURNS:
 * - Absolute filesystem path
 *
 * BEHAVIOR:
 * - Handles root directive: /images/cat.jpg -> <root>/images/cat.jpg
 * - Handles alias directive: /photos/cat.jpg -> <alias>/cat.jpg (drops /photos)
 */
std::string resolvePath(const std::string &uri,
                        const std::string &location_path,
                        const LocationConfig &location);

/**
 * readFile() - Reads entire file into string
 *
 * PARAMETERS:
 * - path: Filesystem path to file
 *
 * RETURNS:
 * - File contents, or empty string if file cannot be read
 */
std::string readFile(const std::string &path);

/**
 * fileExists() - Checks if file exists and is readable
 *
 * PARAMETERS:
 * - path: Filesystem path to check
 *
 * RETURNS:
 * - true if file exists, false otherwise
 */
bool fileExists(const std::string &path);

/**
 * isDirectory() - Checks if path is a directory
 *
 * PARAMETERS:
 * - path: Filesystem path to check
 *
 * RETURNS:
 * - true if path is a directory, false otherwise
 */
bool isDirectory(const std::string &path);

/**
 * findIndexFile() - Searches for index files in directory
 *
 * PARAMETERS:
 * - directory_path: Directory to search
 * - location: Configuration with index file list
 *
 * RETURNS:
 * - Path to first existing index file, or empty string if none found
 *
 * BEHAVIOR:
 * - Checks each file in location.index list (e.g., index.html, index.htm)
 * - Returns first match
 */
std::string findIndexFile(const std::string &directory_path,
                          const LocationConfig &location);

/**
 * getMimeType() - Determines MIME type from file extension
 *
 * PARAMETERS:
 * - path: File path
 * - location: Configuration with MIME type mappings
 *
 * RETURNS:
 * - MIME type string (e.g., "text/html", "image/jpeg")
 * - Falls back to location.default_type if extension not found
 */
const std::string &getMimeType(const std::string &path,
                                const LocationConfig &location);

/**
 * normalizePath() - Normalizes URI path and prevents path traversal
 *
 * PARAMETERS:
 * - path: URI path to normalize
 *
 * RETURNS:
 * - Normalized path with '.', '..' resolved, or empty string if path traversal detected
 *
 * BEHAVIOR:
 * - Removes '.' and '..' segments
 * - Prevents escaping the root directory
 * - Returns empty string if path traversal is detected (e.g., /../../../etc/passwd)
 */
std::string normalizePath(const std::string &path);

} // namespace utils
} // namespace routing
