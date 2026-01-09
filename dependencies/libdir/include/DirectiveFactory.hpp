/**
 * @file DirectiveFactory.hpp
 * @brief Factory class for creating and managing directive registration
 */

#ifndef DIRECTIVEFACTORY_HPP
#define DIRECTIVEFACTORY_HPP

#include <string>
#include <map>
#include <vector>
#include <typeinfo>

// Forward declaration
class IDirective;

/**
 * @class DirectiveFactory
 * @brief Factory class for directive creation and registration
 * 
 * This class manages the registry of all directive types and provides
 * factory methods to create directive instances by name or type.
 * 
 * Design Pattern: Factory + Registry
 * - Separates directive registration logic from directive implementation
 * - Provides centralized directive management
 * - Enables dynamic directive creation 
 */
class DirectiveFactory
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
	 * @brief Registers a directive type in the factory system
	 * @param name String name for the directive
	 * @param id Type info pointer for the directive class
	 * @param creator Factory function to create instances
	 * @note Typically called automatically via DEFINE_DIRECTIVE macro
	 */
	static void registerDirective(const std::string &name, 
	                               const std::type_info *id, 
	                               CreatorFunc creator);

	/**
	 * @brief Gets creator function for directive by name
	 * @param name String name of the directive
	 * @return Factory function pointer
	 * @throws std::out_of_range if name not registered
	 */
	static CreatorFunc getCreator(const std::string &name);

	/**
	 * @brief Gets creator function for directive by type info
	 * @param id Type info pointer for the directive class
	 * @return Factory function pointer
	 * @throws std::out_of_range if type not registered
	 */
	static CreatorFunc getCreator(const std::type_info *id);

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
	static const std::type_info *getTypeInfo(const std::string &name);

private:
	/**
	 * @brief Registry mapping directive names to creators and type info
	 */
	static std::map<std::string, std::pair<CreatorFunc, const std::type_info *> > registry_name;

	/**
	 * @brief Registry mapping type info to creators and names
	 */
	static std::map<const std::type_info *, std::pair<CreatorFunc, std::string> > registry_id;
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
 * 2. Calls the directive's static init##_class(args) method for parsing and construction
 * 3. Creates a registrar struct whose constructor auto-registers the directive
 * 4. Generates the getAllowedInside() implementation
 * 
 * The registrar uses the "static initialization" trick: its constructor
 * runs before main(), automatically registering the directive.
 * 
 * IMPORTANT: Your directive class MUST implement a static init function:
 * @code
 *   static _class* init##_class(const std::vector<std::string>& args);
 * @endcode
 * 
 * Example Usage:
 * @code
 *   // In ServerDirective.hpp:
 *   class ServerDirective : public IDirective {
 *   private:
 *       const std::string ip;
 *       const int port;
 *   public:
 *       ServerDirective(const std::string& ip, int port);
 *       static ServerDirective* initServerDirective(const std::vector<std::string>& args);
 *       DEFINE_ALLOWED_INSIDE();
 *   };
 *   
 *   // In ServerDirective.cpp:
 *   DEFINE_DIRECTIVE(ServerDirective, "server", HttpDirective, GlobalDirective)
 *   
 *   ServerDirective* ServerDirective::initServerDirective(const std::vector<std::string>& args) {
 *       // args[0] = "server"
 *       // args[1] = ip address
 *       // args[2] = port number
 *       std::string ip = args[1];
 *       int port = std::atoi(args[2].c_str());
 *       return new ServerDirective(ip, port);
 *   }
 *   
 *   ServerDirective::ServerDirective(const std::string& ip, int port) 
 *       : ip(ip), port(port) {
 *       // Constructor with properly typed parameters
 *   }
 * @endcode
 * 
 * The macro expands to:
 * @code
 *   static IDirective* createServerDirective(const std::vector<std::string>& args) {
 *       return ServerDirective::initServerDirective(args);
 *   }
 *   struct ServerDirectiveRegistrar {
 *       ServerDirectiveRegistrar() {
 *           DirectiveFactory::registerDirective("server", &typeid(ServerDirective), &createServerDirective);
 *       }
 *   } _ServerDirectiveRegistrar;
 *   // ... plus getAllowedInside() implementation
 * @endcode
 */
#define DEFINE_DIRECTIVE(_class, _name, ...)                                  \
	static IDirective *create##_class(const std::vector<std::string> &args)   \
	{                                                                         \
		return _class::init##_class(args);                                    \
	}                                                                         \
	struct _class##Registrar                                                  \
	{                                                                         \
		_class##Registrar()                                                   \
		{                                                                     \
			DirectiveFactory::registerDirective(_name, &typeid(_class), &create##_class); \
		}                                                                     \
	} _##_class##Registrar;                                                   \
	ALLOWED_INSIDE(_class, _allowedInsert, __VA_ARGS__)

#endif // DIRECTIVEFACTORY_HPP
