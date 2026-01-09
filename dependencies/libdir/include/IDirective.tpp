#include <vector>
#include "IDirective.hpp"

/**
 * @brief Gets all children of a specific type by name
 * @tparam T The directive type to retrieve and cast to
 * @param name The directive name to search for
 * @return Vector of typed directive pointers
 * @throws DirectiveTypeMismatch if dynamic_cast fails
 * 
 * This template method provides type-safe access to child directives.
 * It uses dynamic_cast to ensure type safety at runtime.
 */
template <typename T>
const std::vector<T *> IDirective::getChildren() const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::string name = getName(&typeid(T));

	std::vector<T *> result;

	std::pair<iteratorType, iteratorType> range = children.equal_range(name);

	iteratorType it;

	for (it = range.first; it != range.second; ++it)
	{
		T *casted = dynamic_cast<T *>(it->second);
		if (!casted)
		{
			throw DirectiveTypeMismatch("Expected '" + name + "' but found different type");
		}
		result.push_back(casted);
	}

	return result;
}

/**
 * @brief Gets the first child directive of a specific type
 * @tparam T The directive type to retrieve
 * @return Pointer to first matching directive
 * @throws DirectiveNotFound if no matching directive exists
 * @throws DirectiveTypeMismatch if dynamic_cast fails
 */
template <typename T>
inline const T *IDirective::getFirstChild() const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::string name = getName(&typeid(T));

	iteratorType it = children.find(name);

	if (it == children.end())
		throw DirectiveNotFound(name);

	T *casted = dynamic_cast<T *>(it->second);
	if (!casted)
		throw DirectiveTypeMismatch("First child of '" + name + "' has incorrect type");

	return casted;
}

/**
 * @brief Gets the last child directive of a specific type
 * @tparam T The directive type to retrieve
 * @return Pointer to last matching directive
 * @throws DirectiveNotFound if no matching directive exists
 * @throws DirectiveTypeMismatch if dynamic_cast fails
 */
template <typename T>
inline const T *IDirective::getLastChild() const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::string name = getName(&typeid(T));

	std::pair<iteratorType, iteratorType> range = children.equal_range(name);

	if (range.first == range.second)
		throw DirectiveNotFound(name);

	iteratorType last_it = range.second;
	last_it--;

	T *casted = dynamic_cast<T *>(last_it->second);
	if (!casted)
		throw DirectiveTypeMismatch("Last child of '" + name + "' has incorrect type");

	return casted;
}

/**
 * @brief Gets all children of a specific type wrapped in ListDirectives
 * @tparam T The directive type
 * @return ListDirectives container with all matching directives
 */
template <typename T>
inline const ListDirectives<T> IDirective::getMany() const
{
	std::vector<T *> children = getChildren<T>();
	ListDirectives<T> directives = ListDirectives<T>(children);
	return directives;
}

/**
 * @brief Convenience method to get either first or last child
 * @tparam T The directive type
 * @param first True for first child, false for last
 * @return Pointer to requested directive
 */
template <typename T>
inline const T *IDirective::getOne(bool first) const
{
	if (first)
		return getFirstChild<T>();
	else
		return getLastChild<T>();
}
