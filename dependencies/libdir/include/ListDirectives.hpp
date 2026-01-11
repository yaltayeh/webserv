#include <vector>

#pragma once

namespace Config
{

class IDirective;

template <typename T>
class ListDirectives
{
private:
	std::vector<T *> directives;

public:
	ListDirectives();
	ListDirectives(const std::vector<T *> directives);
	~ListDirectives();

	const T *operator[](size_t index) const;
	size_t size() const;
};

} // namespace Config

#include "ListDirectives.tpp"
