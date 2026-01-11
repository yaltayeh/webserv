#pragma once

#include <iostream>
#include <string>

namespace Directive
{
	/**
	 * @enum LogLevel
	 * @brief Logging levels for debug output control
	 */
	enum LogLevel
	{
		LOG_LEVEL_ERROR = 0,    // Errors only
		LOG_LEVEL_WARN = 1,     // Warnings and errors
		LOG_LEVEL_INFO = 2,     // Info, warnings, and errors
		LOG_LEVEL_DEBUG = 3,    // All messages including debug
		LOG_LEVEL_NONE = -1     // Disable all logging
	};

	/**
	 * @brief Global log level variable
	 * @note Set this to control logging verbosity
	 */
	extern LogLevel g_log_level;
}

// ANSI color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_BOLD    "\033[1m"

/**
 * @def LOGE(msg)
 * @brief Log error message (always visible unless LOG_LEVEL_NONE)
 * @note Displayed in red color
 */
#define LOGE(msg) \
	do { \
		if (Directive::g_log_level >= Directive::LOG_LEVEL_ERROR) { \
			std::cerr << COLOR_RED << COLOR_BOLD << "[ERROR] " << COLOR_RESET \
					  << COLOR_RED << msg << COLOR_RESET << std::endl; \
		} \
	} while (0)

/**
 * @def LOGW(msg)
 * @brief Log warning message (visible at LOG_LEVEL_WARN and above)
 * @note Displayed in yellow color
 */
#define LOGW(msg) \
	do { \
		if (Directive::g_log_level >= Directive::LOG_LEVEL_WARN) { \
			std::cerr << COLOR_YELLOW << COLOR_BOLD << "[WARN] " << COLOR_RESET \
					  << COLOR_YELLOW << msg << COLOR_RESET << std::endl; \
		} \
	} while (0)

/**
 * @def LOGI(msg)
 * @brief Log info message (visible at LOG_LEVEL_INFO and above)
 * @note Displayed in cyan color
 */
#define LOGI(msg) \
	do { \
		if (Directive::g_log_level >= Directive::LOG_LEVEL_INFO) { \
			std::cout << COLOR_CYAN << COLOR_BOLD << "[INFO] " << COLOR_RESET \
					  << COLOR_CYAN << msg << COLOR_RESET << std::endl; \
		} \
	} while (0)

/**
 * @def LOGD(msg)
 * @brief Log debug message (visible only at LOG_LEVEL_DEBUG)
 * @note Displayed in gray color
 */
#define LOGD(msg) \
	do { \
		if (Directive::g_log_level >= Directive::LOG_LEVEL_DEBUG) { \
			std::cout << COLOR_GRAY << "[DEBUG] " << msg << COLOR_RESET << std::endl; \
		} \
	} while (0)

/**
 * @def LOGD_FMT(fmt, ...)
 * @brief Log debug message with format (C++98 compatible)
 * @note Use for debugging with multiple values
 * 
 * Example:
 * @code
 *   LOGD("Parsing directive: " << directive_name << " with " << args.size() << " args");
 * @endcode
 */
#define LOGD_FMT(msg) LOGD(msg)
#define LOGI_FMT(msg) LOGI(msg)
#define LOGW_FMT(msg) LOGW(msg)
#define LOGE_FMT(msg) LOGE(msg)
