/**
 * @file Exceptions.hpp
 * @brief Custom exception classes for the configuration parser library
 * 
 * This file defines a hierarchy of exception classes used throughout the
 * configuration parsing system. All exceptions inherit from std::runtime_error
 * to maintain compatibility with standard C++ exception handling.
 */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace Config
{

/**
 * @class ConfigException
 * @brief Base exception class for all configuration-related errors
 * 
 * This is the root of the exception hierarchy. Catching this type will
 * catch all configuration-related exceptions.
 */
class ConfigException : public std::runtime_error
{
public:
	explicit ConfigException(const std::string &message)
		: std::runtime_error(message) {}
	virtual ~ConfigException() throw() {}
};

/**
 * @class ParseException
 * @brief Exception thrown when parsing configuration text fails
 * 
 * Used for syntax errors, malformed directives, and other parsing issues.
 * Examples: missing semicolons, unmatched brackets, invalid characters.
 */
class ParseException : public ConfigException
{
public:
	explicit ParseException(const std::string &message)
		: ConfigException("Parse error: " + message) {}
	virtual ~ParseException() throw() {}
};

/**
 * @class DirectiveException
 * @brief Exception thrown for directive-related errors
 * 
 * Used when directive operations fail, such as unknown directive names,
 * type mismatches, or validation failures.
 */
class DirectiveException : public ConfigException
{
public:
	explicit DirectiveException(const std::string &message)
		: ConfigException("Directive error: " + message) {}
	virtual ~DirectiveException() throw() {}
};

/**
 * @class DirectiveNotFound
 * @brief Exception thrown when a requested directive doesn't exist
 * 
 * Used when trying to access a child directive that hasn't been defined
 * or when looking up a directive type that isn't registered.
 */
class DirectiveNotFound : public DirectiveException
{
public:
	explicit DirectiveNotFound(const std::string &directiveName)
		: DirectiveException("Directive '" + directiveName + "' not found") {}
	virtual ~DirectiveNotFound() throw() {}
};

/**
 * @class DirectiveTypeMismatch
 * @brief Exception thrown when directive type casting fails
 * 
 * Used when dynamic_cast fails during directive retrieval, indicating
 * the requested type doesn't match the actual directive type.
 */
class DirectiveTypeMismatch : public DirectiveException
{
public:
	explicit DirectiveTypeMismatch(const std::string &message)
		: DirectiveException("Type mismatch: " + message) {}
	virtual ~DirectiveTypeMismatch() throw() {}
};

/**
 * @class InvalidNesting
 * @brief Exception thrown when a directive is placed in an invalid context
 * 
 * Used when trying to nest a directive inside a parent where it's not
 * allowed according to the configuration schema.
 */
class InvalidNesting : public DirectiveException
{
private:
	std::string childType;
	std::string parentType;

public:
	InvalidNesting(const std::string &child, const std::string &parent)
		: DirectiveException("Cannot place '" + child + "' inside '" + parent + "' block"),
		  childType(child),
		  parentType(parent) {}
	virtual ~InvalidNesting() throw() {}

	const std::string &getChildType() const { return childType; }
	const std::string &getParentType() const { return parentType; }
};

/**
 * @class ValidationException
 * @brief Exception thrown when directive validation fails
 * 
 * Used when directive arguments don't meet validation requirements,
 * such as incorrect argument count or invalid argument values.
 */
class ValidationException : public DirectiveException
{
public:
	explicit ValidationException(const std::string &message)
		: DirectiveException("Validation failed: " + message) {}
	virtual ~ValidationException() throw() {}
};

/**
 * @class ArgumentException
 * @brief Exception thrown when init function receives invalid arguments
 * 
 * This exception should be thrown from init methods when:
 * - Required arguments are missing (args.size() too small)
 * - Argument values are invalid (out of range, wrong format, etc.)
 * - Argument types cannot be parsed (invalid number format, etc.)
 * 
 * The parser will catch this and display the error message to the user.
 * 
 * Example:
 * @code
 *   if (args.size() < 2) {
 *       throw ArgumentException("listen", "requires at least 1 argument (port)");
 *   }
 *   int port = std::atoi(args[1].c_str());
 *   if (port < 1 || port > 65535) {
 *       throw ArgumentException("listen", "port must be between 1 and 65535");
 *   }
 * @endcode
 */
class ArgumentException : public DirectiveException
{
private:
	std::string directiveName;

public:
	ArgumentException(const std::string &directive, const std::string &message)
		: DirectiveException("Directive '" + directive + "': " + message),
		  directiveName(directive) {}
	virtual ~ArgumentException() throw() {}

	const std::string &getDirectiveName() const { return directiveName; }
};

/**
 * @class SyntaxException
 * @brief Exception thrown for syntax errors in configuration files
 * 
 * Used for low-level syntax issues like unmatched quotes, invalid
 * escape sequences, or malformed tokens.
 */
class SyntaxException : public ParseException
{
private:
	size_t lineNumber;
	size_t columnNumber;

public:
	SyntaxException(const std::string &message, size_t line = 0, size_t column = 0)
		: ParseException(message),
		  lineNumber(line),
		  columnNumber(column) {}
	virtual ~SyntaxException() throw() {}

	size_t getLine() const { return lineNumber; }
	size_t getColumn() const { return columnNumber; }
};

/**
 * @class FileException
 * @brief Exception thrown for file I/O errors
 * 
 * Used when configuration files cannot be opened, read, or
 * when file operations fail.
 */
class FileException : public ConfigException
{
private:
	std::string filename;

public:
	FileException(const std::string &file, const std::string &message)
		: ConfigException("File error (" + file + "): " + message),
		  filename(file) {}
	virtual ~FileException() throw() {}

	const std::string &getFilename() const { return filename; }
};

} // namespace Config

#endif // EXCEPTIONS_HPP
