#include "../IDirective.hpp"

#pragma once

namespace Config
{

/**
 * @class GlobalDirective
 * @brief Root-level directive representing the global configuration scope
 * 
 * This directive serves as the top-level container for all configuration
 * directives. It has no valid parent contexts (can only exist at root).
 * 
 * All configuration parsing starts with a GlobalDirective instance:
 * @code
 *   GlobalDirective* root = new GlobalDirective();
 *   // Parse directives into root...
 *   delete root;  // Automatically cleans up all children
 * @endcode
 * 
 * Characteristics:
 * - Cannot be nested inside other directives
 * - Serves as the root of the directive tree
 * - Other directives specify GlobalDirective as an allowed parent
 */
class GlobalDirective: public IDirective
{
private:
public:
	/**
	 * @brief Default constructor
	 */
	GlobalDirective();
	
	/**
	 * @brief Copy constructor
	 * @param other GlobalDirective to copy from
	 */
	GlobalDirective(const GlobalDirective &other);
	
	/**
	 * @brief Assignment operator
	 * @param other GlobalDirective to assign from
	 * @return Reference to this
	 */
	GlobalDirective &operator=(const GlobalDirective &other);
	
	/**
	 * @brief Destructor - cleans up all child directives
	 */
	~GlobalDirective();

	/**
	 * @brief Returns "global" as the directive name
	 * @return String "global"
	 */
	std::string getName() const;

	/**
	 * @brief Declares getAllowedInside() method
	 * @return NULL array (no valid parents for root directive)
	 */
	DEFINE_ALLOWED_INSIDE();
};
} // namespace Config
