#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <typeinfo>
#include "ListDirectives.hpp"
#include "Exceptions.hpp"
#include "DirectiveFactory.hpp"

#pragma once

namespace Config
{

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
	 * @brief Copy constructor
	 * @param other IDirective to copy from
	 * @note Performs deep copy of children
	 */
	IDirective(const IDirective &other);
	
	/**
	 * @brief Assignment operator
	 * @param other IDirective to assign from
	 * @return Reference to this
	 * @note Performs deep copy of children
	 */
	IDirective &operator=(const IDirective &other);
	
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


};

} // namespace Config

#include "IDirective.tpp"
