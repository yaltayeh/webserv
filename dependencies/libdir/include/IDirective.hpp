#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <typeinfo>
#include "ListDirectives.hpp"
#include "Exceptions.hpp"

#pragma once

/**
 * @class IDirective
 * @brief Abstract base class for all configuration directives
 * 
 * This class implements the Composite pattern for hierarchical configuration parsing.
 * Each directive can contain child directives and has validation rules for where it
 * can appear in the configuration hierarchy.
 * 
 * Memory Management:
 * - Parent directives own their children
 * - Destructor recursively deletes all children
 * - Use with care when sharing directive pointers
 * 
 * Example Usage:
 * @code
 *   GlobalDirective* root = new GlobalDirective();
 *   Config::Parser parser("server { listen 8080; }");
 *   while (parser.getNextDirectives()) {
 *       IDirective::parseDirective(root, parser.copyDirective());
 *   }
 *   delete root; // Automatically deletes all children
 * @endcode
 */
class IDirective
{
public:
	/**
	 * @typedef CreatorFunc
	 * @brief Function pointer type for directive factory functions
	 * 
	 * Factory functions create directive instances from argument vectors.
	 * Used by the registration system for dynamic directive creation.
	 */
	typedef IDirective *(*CreatorFunc)(const std::vector<std::string> &args);

	/**
	 * @brief Default constructor
	 */
	IDirective();
	
	/**
	 * @brief Constructor with arguments
	 * @param args Vector of string arguments from configuration file
	 * @note First argument (args[0]) is typically the directive name
	 */
	IDirective(const std::vector<std::string> &args);
	
	/**
	 * @brief Virtual destructor
	 * @note Recursively deletes all child directives to prevent memory leaks
	 */
	virtual ~IDirective() = 0;

	/**
	 * @brief Sets the parent directive with validation
	 * @param parent Pointer to the parent directive
	 * @throws InvalidNesting if this directive cannot be nested inside parent
	 * 
	 * Validates that this directive type is allowed inside the parent type
	 * according to the schema defined by getAllowedInside().
	 */
	void setParent(IDirective *parent);
	
	/**
	 * @brief Adds a child directive to this directive
	 * @param child Pointer to the child directive to add
	 * @note Automatically validates nesting rules via setParent()
	 * @note Takes ownership of the child pointer
	 */
	void addChild(IDirective *child);

	/**
	 * @brief Gets all child directives
	 * @return Multimap of directive name to directive pointer
	 * @note Multiple children can have the same name (e.g., multiple 'location' blocks)
	 */
	const std::multimap<std::string, IDirective *> getChildren() const;

	/**
	 * @brief Gets children of specific type with type safety
	 * @tparam T The directive type to retrieve
	 * @return Vector of pointers to directives of type T
	 * @throws DirectiveTypeMismatch if a child has wrong type
	 */
	template <typename T>
	const std::vector<T *> getChildren() const;
	
	/**
	 * @brief Gets the first child of a specific type
	 * @tparam T The directive type to retrieve
	 * @return Pointer to the first directive of type T
	 * @throws DirectiveNotFound if no such directive exists
	 * @throws DirectiveTypeMismatch if type cast fails
	 */
	template <typename T>
	const T * getFirstChild() const;
	
	/**
	 * @brief Gets the last child of a specific type
	 * @tparam T The directive type to retrieve
	 * @return Pointer to the last directive of type T
	 * @throws DirectiveNotFound if no such directive exists
	 * @throws DirectiveTypeMismatch if type cast fails
	 */
	template <typename T>
	const T * getLastChild() const;
	
	/**
	 * @brief Gets all children of a specific type in a list wrapper
	 * @tparam T The directive type to retrieve
	 * @return ListDirectives wrapper containing all matching directives
	 */
	template <typename T>
	const ListDirectives<T> getMany() const;
	
	/**
	 * @brief Convenience method to get first or last child
	 * @tparam T The directive type to retrieve
	 * @param first If true, returns first child; if false, returns last
	 * @return Pointer to the requested directive
	 */
	template <typename T>
	const T *getOne(bool first = true) const;

	/**
	 * @brief Gets the name of this directive
	 * @return String name of the directive (e.g., "server", "location")
	 */
	std::string getName() const;
	
	/**
	 * @brief Gets directive name from type info
	 * @param id Type info pointer for the directive class
	 * @return String name of the directive
	 * @throws std::out_of_range if type not registered
	 */
	static std::string getName(const std::type_info *id);
	
	/**
	 * @brief Gets type info from directive name
	 * @param name String name of the directive
	 * @return Type info pointer for the directive class
	 * @throws std::out_of_range if name not registered
	 */
	static const std::type_info *getID(const std::string &name);
	
	/**
	 * @brief Gets array of allowed parent types
	 * @return NULL-terminated array of type_info pointers
	 * @note Must be implemented by derived classes
	 * @note Return array with single NULL for root-level directives
	 */
	virtual const std::type_info **getAllowedInside() const = 0;

	/**
	 * @brief Parses a directive string and adds it to parent
	 * @param parent Parent directive to add parsed directive to
	 * @param str String containing the directive to parse
	 * @return Pointer to newly created directive, or NULL on error
	 * @throws DirectiveNotFound if directive type unknown
	 * @throws ParseException on syntax errors
	 */
	static const IDirective *parseDirective(IDirective *parent, const std::string &str);
	
	/**
	 * @brief Registers a directive type in the factory system
	 * @param name String name for the directive
	 * @param id Type info pointer for the directive class
	 * @param creator Factory function to create instances
	 * @note Typically called automatically via DEFINE_DIRECTIVE macro
	 */
	static void addRegistry(std::string name, const std::type_info *id, const IDirective::CreatorFunc &creator);
	
	/**
	 * @brief Gets creator function for directive by name
	 * @param name String name of the directive
	 * @return Factory function pointer
	 * @throws std::out_of_range if name not registered
	 */
	static IDirective::CreatorFunc getCreator(std::string name);

	/**
	 * @brief Gets creator function for directive by type info
	 * @param id Type info pointer for the directive class
	 * @return Factory function pointer
	 * @throws std::out_of_range if type not registered
	 */
	static CreatorFunc getCreator(const std::type_info *id);

private:
	/**
	 * @brief Map of child directives by name
	 * @note Using multimap allows multiple children with same name
	 * @note Parent owns these pointers and deletes them in destructor
	 */
	std::multimap<std::string, IDirective *> children;
	
	/**
	 * @brief Pointer to parent directive
	 * @note NULL for root-level directives
	 * @note Does NOT own this pointer
	 */
	const IDirective *parent;

	/**
	 * @brief Extracts directive arguments from text
	 * @param str Full configuration string
	 * @param start Pointer to start of directive in string
	 * @param end Pointer to end of parsing region
	 * @param block_start_ptr Output parameter set to block start if present
	 * @return Vector of argument strings (first is directive name)
	 */
	static std::vector<std::string> getArgs(const std::string &str,
											const char *start,
											const char *end,
											const char **block_start_ptr);

	/**
	 * @brief Internal parsing method with pointer parameters
	 * @param parent Parent directive to add parsed directive to
	 * @param str Full configuration string
	 * @param start Pointer to start of directive
	 * @param end Pointer to end of directive
	 * @return Pointer to created directive or NULL
	 */
	static const IDirective *parseDirective(IDirective *parent,
											const std::string &str,
											const char *start,
											const char *end);

	/**
	 * @brief Recursively parses block directive contents
	 * @param parent Parent directive containing the block
	 * @param str Full configuration string
	 * @param start Pointer to opening brace '{'
	 * @param end Pointer to end of block
	 * @throws ParseException on bracket mismatch
	 */
	static void parseBlockDirective(IDirective *parent,
									const std::string &str,
									const char *start,
									const char *end);

	/**
	 * @brief Registry mapping directive names to creators and type info
	 */
	static std::map<std::string,
					std::pair<CreatorFunc,
							  const std::type_info *> >
		registry_name;
		
	/**
	 * @brief Registry mapping type info to creators and names
	 */
	static std::map<const std::type_info *, std::pair<CreatorFunc,
													  std::string> >
		registry_id;
};

/**
 * @def TYPEID(x)
 * @brief Helper macro to get type_info pointer
 * @param x Type name
 */
#define TYPEID(x) &typeid(x)

/**
 * @def FOREACH_N(x, ...)
 * @brief Recursive macros to handle variable number of type arguments
 * @note C++98 doesn't have variadic templates, so we use macro recursion
 * @note Supports up to 8 parent types
 */
#define FOREACH_1(x, ...) TYPEID(x),
#define FOREACH_2(x, ...) TYPEID(x), FOREACH_1(__VA_ARGS__)
#define FOREACH_3(x, ...) TYPEID(x), FOREACH_2(__VA_ARGS__)
#define FOREACH_4(x, ...) TYPEID(x), FOREACH_3(__VA_ARGS__)
#define FOREACH_5(x, ...) TYPEID(x), FOREACH_4(__VA_ARGS__)
#define FOREACH_6(x, ...) TYPEID(x), FOREACH_5(__VA_ARGS__)
#define FOREACH_7(x, ...) TYPEID(x), FOREACH_6(__VA_ARGS__)
#define FOREACH_8(x, ...) TYPEID(x), FOREACH_7(__VA_ARGS__)

/**
 * @def GET_MACRO
 * @brief Selects appropriate FOREACH_N macro based on argument count
 */
#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME

/**
 * @def ALLOWED_INSIDE_ELEMENTS(...)
 * @brief Expands variadic type list into type_info pointer array
 */
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

/**
 * @def ALLOWED_INSIDE(_class, _allowedInsert, ...)
 * @brief Generates getAllowedInside() implementation
 * @param _class The directive class name
 * @param _allowedInsert Unused placeholder parameter
 * @param ... Variable list of allowed parent types
 * 
 * Example: ALLOWED_INSIDE(ServerDirective, _, HttpDirective, GlobalDirective)
 * Generates method returning array: {&typeid(HttpDirective), &typeid(GlobalDirective), NULL}
 */
#define ALLOWED_INSIDE(_class, _allowedInsert, ...)         \
	const std::type_info **_class::getAllowedInside() const \
	{                                                       \
		static const std::type_info *allowedInside[] = {    \
			ALLOWED_INSIDE_ELEMENTS(__VA_ARGS__)            \
				NULL,                                       \
                                                            \
		};                                                  \
		return allowedInside;                               \
	}

/**
 * @def DEFINE_ALLOWED_INSIDE()
 * @brief Declares the getAllowedInside() method signature
 * @note Used in class declarations
 */
#define DEFINE_ALLOWED_INSIDE() const std::type_info **getAllowedInside() const

/**
 * @def DEFINE_DIRECTIVE(_class, _name, ...)
 * @brief Macro to define and auto-register a directive class
 * @param _class The directive class name
 * @param _name String name for the directive (e.g., "server")
 * @param ... Variable list of allowed parent directive types
 * 
 * This macro:
 * 1. Creates a static factory function create##_class(args)
 * 2. Creates a registrar struct whose constructor auto-registers the directive
 * 3. Generates the getAllowedInside() implementation
 * 
 * The registrar uses the "static initialization" trick: its constructor
 * runs before main(), automatically registering the directive.
 * 
 * Example Usage:
 * @code
 *   // In ServerDirective.cpp:
 *   DEFINE_DIRECTIVE(ServerDirective, "server", HttpDirective, GlobalDirective)
 *   
 *   // This expands to:
 *   static IDirective* createServerDirective(const std::vector<std::string>& args) {
 *       return new ServerDirective(args);
 *   }
 *   struct ServerDirectiveRegistrar {
 *       ServerDirectiveRegistrar() {
 *           IDirective::addRegistry("server", &typeid(ServerDirective), &createServerDirective);
 *       }
 *   } _ServerDirectiveRegistrar;  // Global instance auto-registers
 *   // ... plus getAllowedInside() implementation
 * @endcode
 */
#define DEFINE_DIRECTIVE(_class, _name, ...)                                  \
	static IDirective *create##_class(const std::vector<std::string> &args)   \
	{                                                                         \
		return new _class(args);                                              \
	}                                                                         \
	struct _class##Registrar                                                  \
	{                                                                         \
		_class##Registrar()                                                   \
		{                                                                     \
			IDirective::addRegistry(_name, &typeid(_class), &create##_class); \
		}                                                                     \
	} _##_class##Registrar;                                                   \
	ALLOWED_INSIDE(_class, _allowedInsert, __VA_ARGS__)

#define DEFINE_QUERY_ONE(_class)

#include "IDirective.tpp"
