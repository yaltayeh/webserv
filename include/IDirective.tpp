#include <vector>
#include "IDirective.hpp"

template <typename T>
const std::vector<T *> IDirective::getChildren(const std::string &name) const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::vector<T *> result;

	std::pair<iteratorType, iteratorType> range = children.equal_range(name);

	iteratorType it;

	for (it = range.first; it != range.second; ++it)
	{
		T *casted = dynamic_cast<T *>(it->second);
		if (!casted)
			throw std::runtime_error("getChildren: Child type mismatch"); // TODO
		result.push_back(casted);
	}

	return result;
}

template <typename T>
inline const T *IDirective::getFirstChild() const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::string name = getName(&typeid(T));

	iteratorType it = children.find(name);

	if (it == children.end())
		throw std::runtime_error("getFirstChild: not exist"); // TODO

	T *casted = dynamic_cast<T *>(it->second);
	if (!casted)
		throw std::runtime_error("getFirstChild: Child type mismatch"); // TODO

	return casted;
}

template <typename T>
inline const T *IDirective::getLastChild() const
{
	typedef std::multimap<std::string, IDirective *>::const_iterator iteratorType;

	std::string name = getName(&typeid(T));

	std::pair<iteratorType, iteratorType> range = children.equal_range(name);

	if (range.first == range.second)
		throw std::runtime_error("getLastChild: not exist"); // TODO

	iteratorType last_it = std::prev(range.second);

	T *casted = dynamic_cast<T *>(last_it->second);
	if (!casted)
		throw std::runtime_error("getLastChild: Child type mismatch"); // TODO

	return casted;
}

template <typename T>
inline const ListDirectives<T> IDirective::getMany() const
{
	std::string name = getName(&typeid(T));
	std::vector<T *> children = getChildren<T>(name);
	ListDirectives<T> directives = ListDirectives<T>(children);
	return directives;
}

template <typename T>
inline const T *IDirective::getOne(bool first) const
{
	if (first)
		return getFirstChild<T>();
	else
		return getLastChild<T>();
}
