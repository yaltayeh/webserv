#include "../include/CGI.hpp"
#include "../include/Request.hpp"
#include "../include/Constants.hpp"
#include "../include/Utils.hpp"
#include <StringUtils.hpp>  // from libutils (utils:: namespace) - requires -I flag

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <fcntl.h>
#include <cerrno>

// ============================================================================
// Construction / Destruction
// ============================================================================

CGI::CGI() : _isSetup(false) {
}

CGI::~CGI() {
    cleanupEnvp();
}

// ============================================================================
// Public Interface
// ============================================================================

CGI::SetupResult CGI::setup(const libhttp::Request &request,
                            const std::string &scriptPath,
                            const std::string &interpreterPath,
                            const std::string &serverName,
                            int serverPort) {
    // Clear previous state
    _error.clear();
    _isSetup = false;
    cleanupEnvp();
    _env.clear();

    // Validate inputs first (Fail Fast principle)
    SetupResult validationResult = validateInputs(scriptPath, interpreterPath, serverPort);
    if (validationResult != SETUP_OK) {
        _error = getSetupResultMessage(validationResult);
        return validationResult;
    }

    // Store validated inputs
    _scriptPath = scriptPath;
    _interpreterPath = interpreterPath;

    // Build environment
    buildEnvironment(request, serverName, serverPort);
    prepareEnvp();

    _isSetup = true;
    return SETUP_OK;
}

pid_t CGI::execute(int &readSocketFd, int &writeSocketFd) {
    // Ensure setup was called successfully
    if (!_isSetup) {
        _error = "CGI::execute() called without successful setup()";
        return -1;
    }

    int socketToChild[2];
    int socketFromChild[2];

    if (!createSocketpairs(socketToChild, socketFromChild)) {
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        _error = "Failed to fork: ";
        _error += strerror(errno);
        closeSocketpairs(socketToChild, socketFromChild);
        return -1;
    }

    if (pid == 0) {
        // Child process - this function does not return
        setupChildProcess(socketToChild, socketFromChild);
        _exit(1); // Only reached if setupChildProcess fails
    }

    // Parent process
    close(socketToChild[1]);   // Close child end of write socketpair
    close(socketFromChild[1]); // Close child end of read socketpair

    // Set both sockets to non-blocking mode for non-blocking I/O
    // Note: fcntl errors are not critical - CGI will still work in blocking mode
    // Warnings are stored for debugging purposes
    if (fcntl(socketFromChild[0], F_SETFL, O_NONBLOCK) == -1) {
        if (!_error.empty()) {
            _error += " ; ";
        }
        _error += "Warning: failed to set non-blocking mode on CGI read socket";
    }
    if (fcntl(socketToChild[0], F_SETFL, O_NONBLOCK) == -1) {
        if (!_error.empty()) {
            _error += " ; ";
        }
        _error += "Warning: failed to set non-blocking mode on CGI write socket";
    }

    writeSocketFd = socketToChild[0];
    readSocketFd = socketFromChild[0];

    return pid;
}

const std::string &CGI::getError() const {
    return _error;
}

std::string CGI::getSetupResultMessage(SetupResult result) {
    switch (result) {
        case SETUP_OK:
            return "Setup successful";
        case SETUP_ERROR_EMPTY_SCRIPT_PATH:
            return "Script path cannot be empty";
        case SETUP_ERROR_INVALID_PORT:
            return "Invalid port number (must be 1-65535)";
        case SETUP_ERROR_PATH_TRAVERSAL:
            return "Path traversal detected in script path";
        case SETUP_ERROR_SCRIPT_NOT_FOUND:
            return "Script file not found";
        case SETUP_ERROR_SCRIPT_NOT_EXECUTABLE:
            return "Script file is not executable";
        case SETUP_ERROR_INTERPRETER_NOT_FOUND:
            return "Interpreter not found or not executable";
        default:
            return "Unknown error";
    }
}

// ============================================================================
// Static Methods
// ============================================================================

bool CGI::parseCGIOutput(const std::string &output,
                         std::map<std::string, std::string> &headers,
                         std::string &body) {
    headers.clear();
    body.clear();

    if (output.empty()) {
        return false;
    }

    // Find header/body separator (CRLF or LF)
    size_t headerEnd = output.find("\r\n\r\n");
    size_t separatorLen = 4;
    
    if (headerEnd == std::string::npos) {
        headerEnd = output.find("\n\n");
        separatorLen = 2;
    }

    if (headerEnd == std::string::npos) {
        // No separator - treat as body (non-compliant CGI)
        body = output;
        return false;
    }

    // Parse headers using simple line-by-line approach (KISS)
    size_t pos = 0;
    while (pos < headerEnd) {
        size_t lineEnd = output.find_first_of("\r\n", pos);
        if (lineEnd == std::string::npos || lineEnd > headerEnd) {
            lineEnd = headerEnd;
        }

        std::string line = output.substr(pos, lineEnd - pos);
        
        // Find colon separator
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos && colonPos > 0) {
            // Trim both key and value for liberal parsing (be accepting of malformed headers)
            std::string key = utils::trim(line.substr(0, colonPos));
            std::string value = utils::trim(line.substr(colonPos + 1));
            headers[key] = value;
        }

        // Move to next line
        pos = lineEnd;
        if (pos < headerEnd && output[pos] == '\r') pos++;
        if (pos < headerEnd && output[pos] == '\n') pos++;
    }

    // Extract body
    body = output.substr(headerEnd + separatorLen);
    return true;
}

bool CGI::isCGIScript(const std::string &path, const std::string &extension) {
    return utils::hasExtension(path, extension);
}

// ============================================================================
// Validation (SRP)
// ============================================================================

CGI::SetupResult CGI::validateInputs(const std::string &scriptPath,
                                      const std::string &interpreterPath,
                                      int serverPort) {
    // Check for empty script path
    if (scriptPath.empty()) {
        return SETUP_ERROR_EMPTY_SCRIPT_PATH;
    }

    // Validate port range
    if (!utils::isValidPort(serverPort)) {
        return SETUP_ERROR_INVALID_PORT;
    }

    // Security: Check for path traversal
    if (utils::containsPathTraversal(scriptPath)) {
        return SETUP_ERROR_PATH_TRAVERSAL;
    }
    if (!interpreterPath.empty() && utils::containsPathTraversal(interpreterPath)) {
        return SETUP_ERROR_PATH_TRAVERSAL;
    }

    // Check script exists
    struct stat scriptStat;
    if (stat(scriptPath.c_str(), &scriptStat) != 0) {
        return SETUP_ERROR_SCRIPT_NOT_FOUND;
    }

    // Check interpreter exists if specified
    if (!interpreterPath.empty()) {
        struct stat interpStat;
        if (stat(interpreterPath.c_str(), &interpStat) != 0) {
            return SETUP_ERROR_INTERPRETER_NOT_FOUND;
        }
        // NOTE: Only checks S_IXUSR (user execute permission)
        // Does not check group/other execute permissions
        // This is sufficient for typical CGI setups where scripts are owned by the web server user
        if (!(interpStat.st_mode & S_IXUSR)) {
            return SETUP_ERROR_INTERPRETER_NOT_FOUND;
        }
    } else {
        // Direct execution - script must be executable
        if (!(scriptStat.st_mode & S_IXUSR)) {
            return SETUP_ERROR_SCRIPT_NOT_EXECUTABLE;
        }
    }

    return SETUP_OK;
}

// ============================================================================
// Environment Building (SRP)
// ============================================================================

void CGI::buildEnvironment(const libhttp::Request &request,
                           const std::string &serverName,
                           int serverPort) {
    // Required CGI/1.1 environment variables (RFC 3875)
    
    // Request information
    addEnvironmentVariable("REQUEST_METHOD", libhttp::methodToString(request.getMethod()));
    addEnvironmentVariable("REQUEST_URI", request.getUri());
    
    // URI components
    std::string scriptName = extractScriptName(request.getUri());
    addEnvironmentVariable("SCRIPT_NAME", scriptName);
    addEnvironmentVariable("SCRIPT_FILENAME", _scriptPath);
    addEnvironmentVariable("QUERY_STRING", extractQueryString(request.getUri()));
    
    std::string pathInfo = extractPathInfo(request.getUri(), scriptName);
    addEnvironmentVariable("PATH_INFO", pathInfo);
    
    // Document root (NOTE: Currently set to script directory.
    // In production, this should be the web server's document root configured separately)
    std::string documentRoot = utils::getDirectory(_scriptPath);
    addEnvironmentVariable("DOCUMENT_ROOT", documentRoot);
    
    // PATH_TRANSLATED per RFC 3875 section 4.1.6: DOCUMENT_ROOT + PATH_INFO
    if (!pathInfo.empty()) {
        std::string pathTranslated = documentRoot;
        // Ensure proper path separator between documentRoot and pathInfo
        if (!documentRoot.empty() && documentRoot[documentRoot.length() - 1] != '/') {
            pathTranslated += '/';
        }
        // Remove leading slash from pathInfo if present to avoid double slashes
        if (pathInfo[0] == '/') {
            pathTranslated += pathInfo.substr(1);
        } else {
            pathTranslated += pathInfo;
        }
        addEnvironmentVariable("PATH_TRANSLATED", pathTranslated);
    }

    // Server information
    addEnvironmentVariable("SERVER_NAME", serverName);
    addEnvironmentVariable("SERVER_PORT", utils::intToString(serverPort));
    addEnvironmentVariable("SERVER_PROTOCOL", "HTTP/1.0");
    // RFC 3875 section 4.1.16: SERVER_SOFTWARE = product-name/version
    addEnvironmentVariable("SERVER_SOFTWARE", "Webserv/1.0");
    addEnvironmentVariable("GATEWAY_INTERFACE", "CGI/1.1");
    
    // TODO: When authentication is implemented, add the following meta-variables:
    // - AUTH_TYPE (RFC 3875 section 4.1.18)
    // - REMOTE_USER (RFC 3875 section 4.1.11)
    // - REMOTE_IDENT (RFC 3875 section 4.1.10)
    
    // TODO: Add client connection information:
    // - REMOTE_ADDR (client IP address, RFC 3875 section 4.1.8)
    // - REMOTE_HOST (client hostname if available, RFC 3875 section 4.1.9)

    // Content headers (special handling per CGI spec)
    std::string contentType = request.getHeader("Content-Type");
    if (!contentType.empty()) {
        addEnvironmentVariable("CONTENT_TYPE", contentType);
    }

    std::string contentLength = request.getHeader("Content-Length");
    if (!contentLength.empty()) {
        addEnvironmentVariable("CONTENT_LENGTH", contentLength);
    } else {
        size_t bodyLength = request.getBody().getContentLength();
        addEnvironmentVariable("CONTENT_LENGTH", utils::sizeToString(bodyLength));
    }

    // PHP compatibility
    addEnvironmentVariable("REDIRECT_STATUS", "200");

    // HTTP headers -> HTTP_* environment variables
    // Per RFC 3875, multiple headers with the same name should be combined with commas
    std::multimap<std::string, std::string> headers = request.getHeaders();
    std::map<std::string, std::string> combinedHeaders;
    
    for (std::multimap<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        // Skip Content-Type and Content-Length (handled above)
        if (it->first == "Content-Type" || it->first == "Content-Length") {
            continue;
        }
        
        std::string envKey = headerToEnvVar(it->first);
        
        // Combine multiple values with commas per RFC 3875
        if (combinedHeaders.find(envKey) != combinedHeaders.end()) {
            combinedHeaders[envKey] += ", " + it->second;
        } else {
            combinedHeaders[envKey] = it->second;
        }
    }
    
    // Add all combined headers to environment
    for (std::map<std::string, std::string>::const_iterator it = combinedHeaders.begin();
         it != combinedHeaders.end(); ++it) {
        addEnvironmentVariable(it->first, it->second);
    }
}

void CGI::addEnvironmentVariable(const std::string &key, const std::string &value) {
    // Sanitize value to prevent injection attacks
    _env[key] = utils::sanitizeForEnv(value);
}

std::string CGI::headerToEnvVar(const std::string &headerName) const {
    std::string envVar = "HTTP_";
    for (size_t i = 0; i < headerName.length(); ++i) {
        char c = headerName[i];
        if (c == '-') {
            envVar += '_';
        } else {
            envVar += static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }
    }
    return envVar;
}

// ============================================================================
// Environment Array Management
// ============================================================================

void CGI::prepareEnvp() {
    cleanupEnvp();

    for (std::map<std::string, std::string>::const_iterator it = _env.begin();
         it != _env.end(); ++it) {
        std::string envEntry = it->first + "=" + it->second;
        char *envStr = new char[envEntry.length() + 1];
        std::strcpy(envStr, envEntry.c_str());
        _envp.push_back(envStr);
    }
    _envp.push_back(NULL); // Required by execve
}

void CGI::cleanupEnvp() {
    // Skip the last element which is NULL terminator
    for (size_t i = 0; i < _envp.size() && _envp[i] != NULL; ++i) {
        delete[] _envp[i];
    }
    _envp.clear();
}

// ============================================================================
// URI Parsing Helpers (SRP)
// ============================================================================

std::string CGI::extractScriptName(const std::string &uri) const {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        return uri.substr(0, queryPos);
    }
    return uri;
}

std::string CGI::extractQueryString(const std::string &uri) const {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos && queryPos + 1 < uri.length()) {
        return uri.substr(queryPos + 1);
    }
    return "";
}

std::string CGI::extractPathInfo(const std::string &uri, const std::string &scriptName) const {
    // PATH_INFO is the extra path after the script name
    // e.g., /cgi-bin/script.py/extra/path -> /extra/path
    // NOTE: Uses find() which finds the first occurrence of scriptName
    // If scriptName appears multiple times in URI, this may extract incorrect PATH_INFO
    // This is acceptable for typical CGI usage where script names don't repeat in paths

    size_t scriptEnd = uri.find(scriptName);
    if (scriptEnd == std::string::npos) {
        return "";
    }

    size_t pathInfoStart = scriptEnd + scriptName.length();
    if (pathInfoStart >= uri.length()) {
        return "";
    }

    std::string pathInfo = uri.substr(pathInfoStart);
    
    // Remove query string if present
    size_t queryPos = pathInfo.find('?');
    if (queryPos != std::string::npos) {
        pathInfo = pathInfo.substr(0, queryPos);
    }

    return pathInfo;
}

// ============================================================================
// Socketpair Management (SRP)
// ============================================================================

bool CGI::createSocketpairs(int socketToChild[2], int socketFromChild[2]) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketToChild) == -1) {
        _error = "Failed to create socketpair to child: ";
        _error += strerror(errno);
        return false;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketFromChild) == -1) {
        _error = "Failed to create socketpair from child: ";
        _error += strerror(errno);
        close(socketToChild[0]);
        close(socketToChild[1]);
        return false;
    }

    // Set FD_CLOEXEC on all socketpair file descriptors
    // This prevents CGI child processes from inheriting other clients' file descriptors
    // Critical for concurrent CGI execution where multiple children run simultaneously
    fcntl(socketToChild[0], F_SETFD, FD_CLOEXEC);
    fcntl(socketToChild[1], F_SETFD, FD_CLOEXEC);
    fcntl(socketFromChild[0], F_SETFD, FD_CLOEXEC);
    fcntl(socketFromChild[1], F_SETFD, FD_CLOEXEC);

    return true;
}

void CGI::closeSocketpairs(int socketToChild[2], int socketFromChild[2]) {
    close(socketToChild[0]);
    close(socketToChild[1]);
    close(socketFromChild[0]);
    close(socketFromChild[1]);
}

void CGI::setupChildProcess(int socketToChild[2], int socketFromChild[2]) {
    // Close parent ends of socketpairs
    close(socketToChild[0]);
    close(socketFromChild[0]);

    // Redirect stdin
    if (dup2(socketToChild[1], STDIN_FILENO) == -1) {
        _exit(1);
    }
    close(socketToChild[1]);

    // Redirect stdout
    if (dup2(socketFromChild[1], STDOUT_FILENO) == -1) {
        _exit(1);
    }
    close(socketFromChild[1]);
    
    // NOTE: stderr is not redirected per RFC 3875 section 6.2
    // CGI script errors will go to the server's stderr
    // This is acceptable for the current implementation but may need enhancement
    // for production systems that require capturing CGI error output

    // Change to script directory; exit if this fails
    // NOTE: Only root directory is excluded from chdir for security
    // Other sensitive directories (/etc, /sys) should be restricted via file permissions
    std::string scriptDir = utils::getDirectory(_scriptPath);
    if (!scriptDir.empty() && scriptDir != "/") {
        if (chdir(scriptDir.c_str()) == -1) {
            _exit(1);
        }
    }

    // Execute
    // CRITICAL: Declare scriptName here to ensure lifetime extends until execve()
    // Using const_cast on temporary strings causes undefined behavior
    std::string scriptName = utils::getFilename(_scriptPath);
    char *argv[3];
    if (!_interpreterPath.empty()) {
        argv[0] = const_cast<char *>(_interpreterPath.c_str());
        argv[1] = const_cast<char *>(scriptName.c_str());
        argv[2] = NULL;
        execve(_interpreterPath.c_str(), argv, _envp.data());
    } else {
        argv[0] = const_cast<char *>(scriptName.c_str());
        argv[1] = NULL;
        execve(scriptName.c_str(), argv, _envp.data());
    }

    // execve only returns on failure
    _exit(1);
}