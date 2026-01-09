#include "Directives/GlobalDirective.hpp"

/**
 * @brief Default constructor
 */
GlobalDirective::GlobalDirective() {}

/**
 * @brief Destructor
 */
GlobalDirective::~GlobalDirective() {}

/**
 * @brief Returns the name "global"
 * @note GlobalDirective is the root directive type
 */
std::string GlobalDirective::getName() const
{
	return std::string("global");
}

/**
 * @brief Returns NULL array - GlobalDirective has no valid parents
 * @note GlobalDirective can only exist at the root level
 */
const std::type_info **GlobalDirective::getAllowedInside() const
{
	static const std::type_info *allowedInside[] = {NULL};
	return allowedInside;
}
