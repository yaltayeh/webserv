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
			throw std::runtime_error("Child type mismatch");
		result.push_back(casted);
	}

	return result;
}
