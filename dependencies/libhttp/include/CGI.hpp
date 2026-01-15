#pragma once

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include "Request.hpp"

/**
 * @brief CGI (Common Gateway Interface) Handler
 *
 * Executes CGI scripts according to RFC 3875.
 * Features:
 * - Environment variable setup
 * - Non-blocking pipe I/O
 * - Input validation and security checks
 * - Support for various interpreters (Python, PHP, Perl, etc.)
 * 
 * Design Principles:
 * - SRP: Each method has a single responsibility
 * - DRY: Reuses utilities from utils:: namespace (libutils)
 * - KISS: Simple, focused interface
 */
class CGI
{
public:
    /**
     * @brief Result codes for setup validation
     */
    enum SetupResult {
        SETUP_OK = 0,
        SETUP_ERROR_EMPTY_SCRIPT_PATH,
        SETUP_ERROR_INVALID_PORT,
        SETUP_ERROR_PATH_TRAVERSAL,
        SETUP_ERROR_SCRIPT_NOT_FOUND,
        SETUP_ERROR_SCRIPT_NOT_EXECUTABLE,
        SETUP_ERROR_INTERPRETER_NOT_FOUND
    };

    CGI();
    ~CGI();

    /**
     * @brief Prepare CGI execution with validation
     * @param request The HTTP request
     * @param scriptPath Full path to the CGI script
     * @param interpreterPath Path to the interpreter (empty for direct execution)
     * @param serverName Server name for SERVER_NAME env
     * @param serverPort Server port for SERVER_PORT env
     * @return SetupResult indicating success or specific error
     */
    SetupResult setup(const libhttp::Request &request,
                      const std::string &scriptPath,
                      const std::string &interpreterPath,
                      const std::string &serverName,
                      int serverPort);

    /**
     * @brief Execute the CGI script (fork and exec)
     * 
     * Must be called after successful setup().
     * Creates a single bidirectional socketpair for communication with CGI script.
     * 
     * @param readPipeFd Output: socket fd to read CGI output from (same as writePipeFd)
     * @param writePipeFd Output: socket fd to write request body to (same as readPipeFd)
     * @return PID of child process, or -1 on error
     * 
     * @note CRITICAL: The caller MUST close the socket fd after writing the
     *       request body to signal EOF to the CGI script. Since readPipeFd and
     *       writePipeFd are the same bidirectional socket, closing either will
     *       close both. For POST requests, write the body then close. For GET
     *       requests, close immediately after reading the response.
     */
    pid_t execute(int &socketFd);

    /**
     * @brief Get the error message if setup() or execute() failed
     */
    const std::string &getError() const;

    /**
     * @brief Get human-readable description for SetupResult
     */
    static std::string getSetupResultMessage(SetupResult result);

    /**
     * @brief Parse CGI output headers and separate from body
     * @param output Raw CGI output
     * @param headers Output: parsed headers
     * @param body Output: body content
     * @return true if headers parsed successfully
     */
    static bool parseCGIOutput(const std::string &output,
                               std::map<std::string, std::string> &headers,
                               std::string &body);

    /**
     * @brief Check if a file is a CGI script based on extension
     * @param path File path
     * @param extension CGI script extension (e.g., ".cgi", ".pl")
     * @return true if the file is a CGI script
     */
    static bool isCGIScript(const std::string &path, const std::string &extension);

private:
    // State
    std::map<std::string, std::string> _env;
    std::vector<char *> _envp;
    std::string _scriptPath;
    std::string _interpreterPath;
    std::string _requestBody;
    std::string _error;
    bool _isSetup;

    // Validation (SRP: separate validation logic)
    SetupResult validateInputs(const std::string &scriptPath,
                               const std::string &interpreterPath,
                               int serverPort);

    // Environment building (SRP: separate env construction)
    void buildEnvironment(const libhttp::Request &request,
                          const std::string &serverName,
                          int serverPort);
    void addEnvironmentVariable(const std::string &key, const std::string &value);
    std::string headerToEnvVar(const std::string &headerName) const;

    // Environment array management
    void prepareEnvp();
    void cleanupEnvp();

    // URI parsing helpers
    std::string extractScriptName(const std::string &uri) const;
    std::string extractQueryString(const std::string &uri) const;
    std::string extractPathInfo(const std::string &uri, const std::string &scriptName) const;

    void setupChildProcess(int sv[2]);

    // Disabled copy operations
    CGI(const CGI &);
    CGI &operator=(const CGI &);
};
