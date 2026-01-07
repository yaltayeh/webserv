#include <string>
#include <unordered_map>
#include <map>
#include <iostream>

#pragma once

class IDirective
{
private:
	// all directive in block {...}
	std::multimap<const std::string, IDirective *> children;
	const IDirective	*parent;

	static std::vector<std::string> getArgs(const std::string &str,
											const char *start,
											const char *end,
											const char **block_start_ptr);

	static const IDirective *parseDirective(const IDirective *parent,
											const std::string &str,
											const char *start,
											const char *end);

	static void parseBlockDirective(const IDirective *parent,
									const std::string &str,
									const char *start,
									const char *end);

public:
	IDirective();
	IDirective(const std::vector<std::string> &args);
	virtual ~IDirective() = 0;

	static const IDirective *parseDirective(const IDirective *parent, const std::string &str);

	static const std::type_info *context[];

	typedef IDirective *(*CreatorFunc)(const std::vector<std::string> &args);
	static std::unordered_map<std::string, CreatorFunc> registry;
	virtual const std::type_info **getAllowedInside();
};

#define DEFINE_DIRECTIVE(_class, _name)                                     \
	static IDirective *create##_class(const std::vector<std::string> &args) \
	{                                                                       \
		return new _class(args);                                            \
	}                                                                       \
	struct _class##Registrar                                                \
	{                                                                       \
		_class##Registrar()                                                 \
		{                                                                   \
			IDirective::registry[_name] = &create##_class;                  \
		}                                                                   \
	} _##_class##Registrar;

#define TYPEID(x) &typeid(x)

#define FOREACH_1(x, ...) TYPEID(x)
#define FOREACH_2(x, ...) TYPEID(x), FOREACH_1(__VA_ARGS__)
#define FOREACH_3(x, ...) TYPEID(x), FOREACH_2(__VA_ARGS__)
#define FOREACH_4(x, ...) TYPEID(x), FOREACH_3(__VA_ARGS__)
#define FOREACH_5(x, ...) TYPEID(x), FOREACH_4(__VA_ARGS__)
#define FOREACH_6(x, ...) TYPEID(x), FOREACH_5(__VA_ARGS__)
#define FOREACH_7(x, ...) TYPEID(x), FOREACH_6(__VA_ARGS__)
#define FOREACH_8(x, ...) TYPEID(x), FOREACH_7(__VA_ARGS__)

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME

#define ALLOWED_INSIDE_ELEMENTS(...) \
	GET_MACRO(__VA_ARGS__,           \
			  FOREACH_8,             \
			  FOREACH_7,             \
			  FOREACH_6,             \
			  FOREACH_5,             \
			  FOREACH_4,             \
			  FOREACH_3,             \
			  FOREACH_2,             \
			  FOREACH_1)(__VA_ARGS__)

#define ALLOWED_INSIDE(...)                              \
	const std::type_info **getAllowedInside()            \
	{                                                    \
		static const std::type_info *allowedInside[] = { \
			ALLOWED_INSIDE_ELEMENTS(__VA_ARGS__)         \
                                                         \
		};                                               \
		return allowedInside;                            \
	}\
