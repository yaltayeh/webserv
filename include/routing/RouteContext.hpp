/**
 * RouteContext.hpp - Context passed through middleware chain
 *
 * WHAT THIS CLASS DOES:
 * Provides a shared context that middleware can use to communicate
 * and store request-specific data as the request flows through the chain.
 *
 * DESIGN:
 * - Simple data container passed by reference through middleware
 * - Allows middleware to share computed values (resolved paths, metadata)
 * - Lightweight and extensible
 */
#pragma once

#include <string>
#include <map>

namespace routing
{

struct RouteContext
{
	// Resolved filesystem path (computed by path resolution middleware)
	std::string resolvedPath;

	// Arbitrary metadata that middleware can set/read
	std::map<std::string, std::string> metadata;

	// Flag for middleware to signal early termination
	// (not strictly needed with Response* pattern, but useful for clarity)
	bool shouldContinue;

	RouteContext()
		: resolvedPath(""),
		  metadata(),
		  shouldContinue(true)
	{
	}

	// Helper to set/get metadata
	void set(const std::string &key, const std::string &value)
	{
		metadata[key] = value;
	}

	std::string get(const std::string &key) const
	{
		std::map<std::string, std::string>::const_iterator it = metadata.find(key);
		if (it != metadata.end())
			return it->second;
		return "";
	}

	bool has(const std::string &key) const
	{
		return metadata.find(key) != metadata.end();
	}
};

} // namespace routing
