#include "ListDirectives.hpp"

template <typename T>
inline ListDirectives<T>::ListDirectives() {}

template <typename T>
inline ListDirectives<T>::ListDirectives(std::vector<T *> directives) : directives(directives) {}

template <typename T>
inline ListDirectives<T>::~ListDirectives() {}

template <typename T>
inline const T *ListDirectives<T>::operator[](size_t index)
{
	if (index >= directives.size())
		throw std::runtime_error("ListDirectives: index >= directives.size()"); // TODO
	return directives[index];
}

template <typename T>
inline size_t ListDirectives<T>::size() const
{
	return directives.size();
}
