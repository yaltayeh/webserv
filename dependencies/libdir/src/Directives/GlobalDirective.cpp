#include "Directives/GlobalDirective.hpp"

/**
 * @brief Default constructor
 */
GlobalDirective::GlobalDirective() {}

/**
 * @brief Copy constructor
 * @param other GlobalDirective to copy from
 * @note Calls base class copy constructor
 */
GlobalDirective::GlobalDirective(const GlobalDirective &other) : IDirective(other)
{
	// Base class handles children copying
}

/**
 * @brief Assignment operator
 * @param other GlobalDirective to assign from
 * @return Reference to this
 */
GlobalDirective &GlobalDirective::operator=(const GlobalDirective &other)
{
	// Self-assignment check
	if (this == &other)
		return *this;
	
	// Call base class assignment
	IDirective::operator=(other);
	
	return *this;
}

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
