#pragma once

#include <string>
#include <vector>
#include "Config.hpp"

/**
 * @namespace Directive
 * @brief Namespace containing configuration parsing utilities
 */
namespace Directive
{

	/**
	 * @class Parser
	 * @brief Tokenizer for configuration files
	 *
	 * This class provides low-level parsing of configuration text into individual
	 * directive strings. It handles:
	 * - Whitespace and comment skipping
	 * - Quote handling (both single and double quotes)
	 * - Bracket matching for block directives
	 * - Escape sequence processing
	 *
	 * The parser maintains state across calls to getNextDirectives(), allowing
	 * iteration over all directives in a configuration string.
	 *
	 * Example Usage:
	 * @code
	 *   Directive::Parser parser = Directive::Parser::openConfigFile("nginx.conf");
	 *   while (parser.getNextDirectives()) {
	 *       std::string directive = parser.copyDirective();
	 *       // Process directive...
	 *   }
	 * @endcode
	 *
	 * Memory: O(n) where n is the size of the configuration string
	 * Time: O(n) for complete parsing (single pass through input)
	 */
	class Parser
	{
	public:
		typedef Config *(*ParserHandler)(Parser *parser, const std::vector<std::string> &args, Config *parent);

	private:
		/**
		 * @brief Full configuration file content
		 * @note Stored as const to prevent accidental modification
		 */
		const std::string _configString;

		/**
		 * @brief Pointer to start of parsing region
		 */
		const char *_start;

		/**
		 * @brief Pointer to end of parsing region
		 */
		const char *_end;

		/**
		 * @brief Current parsing position
		 * @note Advanced by getNextDirectives()
		 */
		const char *_p;

		/**
		 * @brief Start of current directive
		 * @note Set by getNextDirectives()
		 */
		const char *_directiveStart;

		/**
		 * @brief End of current directive
		 * @note Set by getNextDirectives()
		 */
		const char *_directiveEnd;

		const char *_blockStart;
		const char *_blockEnd;

		ParserHandler _handler;

		std::string _filePath;
		std::string _dirPath;

		// Config *config;

		/**
		 * @brief Extracts directive arguments from text
		 * @param str Full configuration string
		 * @param start Pointer to start of directive in string
		 * @param end Pointer to end of parsing region
		 * @param block_start_ptr Output parameter set to block start if present
		 * @return Vector of argument strings (first is directive name)
		 */
		std::vector<std::string> getArgs();

		/**
		 * @brief Constructor - parses entire string
		 * @param configString Configuration file content
		 */
		Parser(const std::string &configString);

	public:
		/**
		 * @brief Copy constructor
		 * @param other Parser to copy from
		 */
		Parser(const Parser &other);

		/**
		 * @brief Assignment operator
		 * @param other Parser to assign from
		 * @return Reference to this
		 */
		Parser &operator=(const Parser &other);

		/**
		 * @brief Destructor
		 */
		~Parser();

		/**
		 * @brief Constructor - parses substring
		 * @param configString Configuration file content
		 * @param start Starting offset in string
		 * @param len Length of substring to parse
		 * @note Useful for parsing nested blocks
		 */
		Parser(std::string configString, size_t start, size_t len);

		/**
		 * @brief Opens and reads a configuration file
		 * @param fileName Path to configuration file
		 * @return Parser instance with file contents
		 * @throws FileException if file cannot be opened
		 *
		 * This static factory method reads the entire file into memory
		 * and creates a Parser instance.
		 *
		 */
		static Parser openConfigFile(const std::string &fileName);

		static bool loadConfigFile(const std::string &fileName, Config &config);

		void setHandler(ParserHandler handler);
		ParserHandler getHandler() const;

		/**
		 * @brief Recursively parses block directive contents
		 * @param parent Parent directive containing the block
		 * @param str Full configuration string
		 * @param start Pointer to opening brace '{'
		 * @param end Pointer to end of block
		 * @throws ParseException on bracket mismatch
		 */
		void parseBlockDirective(Config *parent);

		/**
		 * @brief Advances to next directive in configuration
		 * @return true if directive found, false if end reached
		 * @throws ParseException on syntax errors (unmatched brackets, etc.)
		 *
		 * This method:
		 * 1. Skips whitespace and comments
		 * 2. Finds the start of the next directive
		 * 3. Determines directive boundaries (';' or '}')
		 * 4. Handles nested blocks with bracket counting
		 * 5. Sets _directiveStart and _directiveEnd pointers
		 *
		 * Call copyDirective() to get the directive text.
		 */
		bool getNextDirectives();

		/**
		 * @brief Parses a directive string and adds it to parent
		 * @param parent Parent directive to add parsed directive to
		 * @param str String containing the directive to parse
		 * @return Pointer to newly created directive, or NULL on error
		 * @throws DirectiveNotFound if directive type unknown
		 * @throws ParseException on syntax errors
		 */
		const Config *parseDirective(Config *parent);

		/**
		 * @brief Gets pointer to start of current directive
		 * @return Pointer into _configString
		 */
		const char *getDirectiveStart() const;

		/**
		 * @brief Gets pointer to end of current directive
		 * @return Pointer into _configString
		 */
		const char *getDirectiveEnd() const;

		const std::string &getFilePath() const;
		const std::string &getDirPath() const;

		/**
		 * @brief Skips whitespace characters
		 * @param p Current position
		 * @param end End boundary
		 * @return Pointer to first non-whitespace character
		 * @note Static utility method usable without Parser instance
		 */
		static const char *skipSpace(const char *p, const char *end);

		/**
		 * @brief Skips comment (from '#' to end of line)
		 * @param p Pointer to '#' character
		 * @param end End boundary
		 * @return Pointer to character after newline
		 * @note Static utility method
		 */
		static const char *skipComment(const char *p, const char *end);

		/**
		 * @brief Advances to closing quote, handling escape sequences
		 * @param p Pointer to opening quote
		 * @param end End boundary
		 * @param qout Quote character (either ' or ")
		 * @return Pointer to closing quote, or NULL if unclosed
		 * @note Now properly handles escape sequences like \\", \\', \\\\
		 * @note Static utility method
		 */
		static const char *nextQout(const char *p, const char *end, char qout);

		/**
		 * @brief Extracts and validates directive name
		 * @param p Current position
		 * @param end End boundary
		 * @param name Output parameter - receives the directive name
		 * @return Pointer past the name, or NULL if invalid
		 * @throws SyntaxException on invalid characters
		 *
		 * Directive names must:
		 * - Start with lowercase letter or underscore
		 * - Contain only lowercase, underscore, or digits
		 * - Not be immediately followed by '{'
		 *
		 * @note Static utility method
		 */
		static const char *getName(const char *p, const char *end, std::string &name);
	};

}
