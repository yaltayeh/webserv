#include "Logger.hpp"

namespace Directive
{
	// Default log level: show errors and warnings only
	// Set to LOG_LEVEL_DEBUG for verbose output during development
	// Set to LOG_LEVEL_NONE to disable all logging in production
	LogLevel g_log_level = LOG_LEVEL_WARN;
}
