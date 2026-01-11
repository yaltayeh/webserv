/**
 * CGI Test Suite
 * 
 * Comprehensive tests for the CGI class implementation.
 * Covers: unit tests, edge cases, security, and integration tests.
 * 
 * Compile with: make test
 */

#include "../dependencies/libhttp/include/CGI.hpp"
#include "../dependencies/libhttp/include/Request.hpp"
#include "../dependencies/libhttp/include/Constants.hpp"
#include "../dependencies/libhttp/include/Utils.hpp"
#include "../dependencies/libutils/include/StringUtils.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>

// Test result counters
static int g_testsRun = 0;
static int g_testsPassed = 0;
static int g_testsFailed = 0;

// ANSI color codes
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) runTest(#name, test_##name)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << COLOR_RED << "  FAILED: " << #condition << " is false" << COLOR_RESET << std::endl; \
            throw std::runtime_error("Assertion failed"); \
        } \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << COLOR_RED << "  FAILED: Expected " << (expected) << " but got " << (actual) << COLOR_RESET << std::endl; \
            throw std::runtime_error("Assertion failed"); \
        } \
    } while (0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::cerr << COLOR_RED << "  FAILED: Expected not " << (expected) << COLOR_RESET << std::endl; \
            throw std::runtime_error("Assertion failed"); \
        } \
    } while (0)

#define ASSERT_CONTAINS(haystack, needle) \
    do { \
        if ((haystack).find(needle) == std::string::npos) { \
            std::cerr << COLOR_RED << "  FAILED: String does not contain '" << (needle) << "'" << COLOR_RESET << std::endl; \
            throw std::runtime_error("Assertion failed"); \
        } \
    } while (0)

typedef void (*TestFunc)();

void runTest(const char* name, TestFunc func) {
    g_testsRun++;
    std::cout << "Running: " << name << "... ";
    std::cout.flush();
    try {
        func();
        std::cout << COLOR_GREEN << "PASSED" << COLOR_RESET << std::endl;
        g_testsPassed++;
    } catch (const std::exception& e) {
        std::cout << COLOR_RED << "FAILED" << COLOR_RESET << std::endl;
        g_testsFailed++;
    }
}

// Helper functions
std::string getCwd() {
    char buffer[4096];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        return std::string(buffer);
    }
    return "";
}

std::string readFromFd(int fd, int timeoutMs = 5000) {
    std::string result;
    char buffer[4096];
    int elapsed = 0;
    const int sleepInterval = 10;

    while (elapsed < timeoutMs) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
            elapsed = 0;
        } else if (bytesRead == 0) {
            break;
        } else {
            // bytesRead == -1: error or would block
            // For non-blocking fd, continue reading after a short delay
            // NOTE: This violates 42 School requirements (no errno checking)
            // but is acceptable for test code to handle non-blocking I/O
            usleep(sleepInterval * 1000);
            elapsed += sleepInterval;
        }
    }
    return result;
}

int waitForChild(pid_t pid, int timeoutMs = 5000) {
    int status;
    int elapsed = 0;
    const int sleepInterval = 10;

    while (elapsed < timeoutMs) {
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid) {
            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        } else if (result == -1) {
            return -1;
        }
        usleep(sleepInterval * 1000);
        elapsed += sleepInterval;
    }
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    return -1;
}

// ============================================================================
// Utils Tests (DRY principle verification)
// ============================================================================

TEST(utils_trim) {
    ASSERT_EQ(std::string("hello"), utils::trim("  hello  "));
    ASSERT_EQ(std::string("hello"), utils::trim("\t\nhello\r\n"));
    ASSERT_EQ(std::string(""), utils::trim("   "));
    ASSERT_EQ(std::string("hello world"), utils::trim("hello world"));
}

TEST(utils_trimLeft) {
    ASSERT_EQ(std::string("hello  "), utils::trimLeft("  hello  "));
    ASSERT_EQ(std::string(""), utils::trimLeft("   "));
}

TEST(utils_trimRight) {
    ASSERT_EQ(std::string("  hello"), utils::trimRight("  hello  "));
    ASSERT_EQ(std::string(""), utils::trimRight("   "));
}

TEST(utils_toUpper) {
    ASSERT_EQ(std::string("HELLO"), utils::toUpper("hello"));
    ASSERT_EQ(std::string("HELLO WORLD"), utils::toUpper("Hello World"));
    ASSERT_EQ(std::string("123ABC"), utils::toUpper("123abc"));
}

TEST(utils_toLower) {
    ASSERT_EQ(std::string("hello"), utils::toLower("HELLO"));
    ASSERT_EQ(std::string("hello world"), utils::toLower("Hello World"));
    ASSERT_EQ(std::string("123abc"), utils::toLower("123ABC"));
}

TEST(utils_getDirectory) {
    ASSERT_EQ(std::string("/path/to"), utils::getDirectory("/path/to/file.txt"));
    ASSERT_EQ(std::string("."), utils::getDirectory("file.txt"));
    ASSERT_EQ(std::string("/"), utils::getDirectory("/file.txt"));  // Root directory case
    ASSERT_EQ(std::string("/"), utils::getDirectory("/etc"));        // Root directory case
}

TEST(utils_getFilename) {
    ASSERT_EQ(std::string("file.txt"), utils::getFilename("/path/to/file.txt"));
    ASSERT_EQ(std::string("script.py"), utils::getFilename("script.py"));
    ASSERT_EQ(std::string("file.txt"), utils::getFilename("/file.txt"));
    ASSERT_EQ(std::string(""), utils::getFilename("/path/to/"));
    ASSERT_EQ(std::string("test"), utils::getFilename("test"));
}

TEST(utils_getExtension) {
    ASSERT_EQ(std::string(".txt"), utils::getExtension("/path/to/file.txt"));
    ASSERT_EQ(std::string(".py"), utils::getExtension("script.py"));
    ASSERT_EQ(std::string(""), utils::getExtension("noextension"));
    ASSERT_EQ(std::string(""), utils::getExtension("/path.dir/noext"));
}

TEST(utils_hasExtension) {
    ASSERT_TRUE(utils::hasExtension("/path/to/script.py", ".py"));
    ASSERT_TRUE(utils::hasExtension("test.cgi", ".cgi"));
    ASSERT_FALSE(utils::hasExtension("script.py", ".cgi"));
    ASSERT_FALSE(utils::hasExtension("short", ".extension"));
    ASSERT_FALSE(utils::hasExtension("file.py", ""));
}

TEST(utils_isValidPort) {
    ASSERT_TRUE(utils::isValidPort(80));
    ASSERT_TRUE(utils::isValidPort(8080));
    ASSERT_TRUE(utils::isValidPort(1));
    ASSERT_TRUE(utils::isValidPort(65535));
    ASSERT_FALSE(utils::isValidPort(0));
    ASSERT_FALSE(utils::isValidPort(-1));
    ASSERT_FALSE(utils::isValidPort(65536));
    ASSERT_FALSE(utils::isValidPort(-8080));
}

TEST(utils_containsPathTraversal) {
    ASSERT_TRUE(utils::containsPathTraversal("../etc/passwd"));
    ASSERT_TRUE(utils::containsPathTraversal("/path/../secret"));
    ASSERT_TRUE(utils::containsPathTraversal(".."));
    ASSERT_TRUE(utils::containsPathTraversal("../"));
    ASSERT_FALSE(utils::containsPathTraversal("/path/to/script.py"));
    ASSERT_FALSE(utils::containsPathTraversal("script.py"));
    ASSERT_FALSE(utils::containsPathTraversal("file..txt")); // Legitimate filename with ".."
    
    // Test null byte detection (security check)
    std::string pathWithNull = std::string("/etc/passwd") + '\0' + "/../safe";
    ASSERT_TRUE(utils::containsPathTraversal(pathWithNull));
}

TEST(utils_sanitizeForEnv) {
    ASSERT_EQ(std::string("hello"), utils::sanitizeForEnv("hello"));
    ASSERT_EQ(std::string("helloworld"), utils::sanitizeForEnv("hello\nworld"));
    ASSERT_EQ(std::string("test"), utils::sanitizeForEnv("test\r\n"));
    std::string withNull = std::string("hel") + '\0' + "lo";
    ASSERT_EQ(std::string("hello"), utils::sanitizeForEnv(withNull));
}

TEST(utils_intToString) {
    ASSERT_EQ(std::string("0"), utils::intToString(0));
    ASSERT_EQ(std::string("123"), utils::intToString(123));
    ASSERT_EQ(std::string("-456"), utils::intToString(-456));
    ASSERT_EQ(std::string("2147483647"), utils::intToString(2147483647)); // INT_MAX
    ASSERT_EQ(std::string("-2147483648"), utils::intToString(-2147483648)); // INT_MIN
}

TEST(utils_sizeToString) {
    ASSERT_EQ(std::string("0"), utils::sizeToString(0));
    ASSERT_EQ(std::string("12345"), utils::sizeToString(12345));
    ASSERT_EQ(std::string("4294967295"), utils::sizeToString(4294967295U)); // Close to max size_t on 32-bit
}

TEST(utils_stringToInt) {
    ASSERT_EQ(123, utils::stringToInt("123"));
    ASSERT_EQ(-456, utils::stringToInt("-456"));
    ASSERT_EQ(0, utils::stringToInt("0"));
    // Test default value on invalid input
    ASSERT_EQ(0, utils::stringToInt(""));
    ASSERT_EQ(-1, utils::stringToInt("", -1));
    ASSERT_EQ(0, utils::stringToInt("abc"));
    ASSERT_EQ(42, utils::stringToInt("xyz", 42));
    ASSERT_EQ(0, utils::stringToInt("123abc")); // Partial parse should fail, return default
    // Test overflow handling
    ASSERT_EQ(0, utils::stringToInt("9999999999999999999"));
    ASSERT_EQ(-1, utils::stringToInt("9999999999999999999", -1));
}

TEST(utils_methodToString) {
    ASSERT_EQ(std::string("GET"), libhttp::methodToString(libhttp::GET));
    ASSERT_EQ(std::string("POST"), libhttp::methodToString(libhttp::POST));
    ASSERT_EQ(std::string("DELETE"), libhttp::methodToString(libhttp::DELETE));
}

// ============================================================================
// isCGIScript Tests
// ============================================================================

TEST(isCGIScript_py_extension) {
    ASSERT_TRUE(CGI::isCGIScript("/path/to/script.py", ".py"));
    ASSERT_TRUE(CGI::isCGIScript("script.py", ".py"));
}

TEST(isCGIScript_cgi_extension) {
    ASSERT_TRUE(CGI::isCGIScript("/path/to/script.cgi", ".cgi"));
}

TEST(isCGIScript_wrong_extension) {
    ASSERT_FALSE(CGI::isCGIScript("/path/to/script.py", ".cgi"));
    ASSERT_FALSE(CGI::isCGIScript("/path/to/script.php", ".py"));
}

TEST(isCGIScript_no_extension) {
    ASSERT_FALSE(CGI::isCGIScript("/path/to/script", ".py"));
}

TEST(isCGIScript_empty_path) {
    ASSERT_FALSE(CGI::isCGIScript("", ".py"));
}

TEST(isCGIScript_empty_extension) {
    ASSERT_FALSE(CGI::isCGIScript("script.py", ""));
}

// ============================================================================
// parseCGIOutput Tests
// ============================================================================

TEST(parseCGIOutput_simple_headers) {
    std::string output = "Content-Type: text/html\r\n\r\n<html>body</html>";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(1u, headers.size());
    ASSERT_EQ(std::string("text/html"), headers["Content-Type"]);
    ASSERT_EQ(std::string("<html>body</html>"), body);
}

TEST(parseCGIOutput_multiple_headers) {
    std::string output = "Content-Type: text/plain\r\nX-Custom: value\r\nStatus: 200 OK\r\n\r\nBody";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(3u, headers.size());
    ASSERT_EQ(std::string("text/plain"), headers["Content-Type"]);
    ASSERT_EQ(std::string("value"), headers["X-Custom"]);
}

TEST(parseCGIOutput_lf_line_endings) {
    std::string output = "Content-Type: text/html\n\n<html>body</html>";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(std::string("text/html"), headers["Content-Type"]);
}

TEST(parseCGIOutput_no_body) {
    std::string output = "Content-Type: text/html\r\n\r\n";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    ASSERT_TRUE(body.empty());
}

TEST(parseCGIOutput_no_headers) {
    std::string output = "Just body content";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_FALSE(result);
    ASSERT_EQ(output, body);
}

TEST(parseCGIOutput_empty_output) {
    std::string output = "";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_FALSE(result);
}

TEST(parseCGIOutput_header_with_spaces) {
    std::string output = "Content-Type:   text/html  \r\n\r\nbody";
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    // Value should have both leading and trailing spaces trimmed per HTTP standards
    ASSERT_EQ(std::string("text/html"), headers["Content-Type"]);
}

TEST(parseCGIOutput_binary_body) {
    std::string output = "Content-Type: application/octet-stream\r\n\r\n";
    output += std::string("\x00\x01\x02\x03", 4);
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool result = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(4u, body.length());
}

// ============================================================================
// Setup Validation Tests (Edge Cases & Security)
// ============================================================================

TEST(setup_empty_script_path) {
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, "", "/usr/bin/python3", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_ERROR_EMPTY_SCRIPT_PATH, result);
    ASSERT_FALSE(cgi.getError().empty());
}

TEST(setup_invalid_port_zero) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 0);
    
    ASSERT_EQ(CGI::SETUP_ERROR_INVALID_PORT, result);
}

TEST(setup_invalid_port_negative) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", -1);
    
    ASSERT_EQ(CGI::SETUP_ERROR_INVALID_PORT, result);
}

TEST(setup_invalid_port_too_high) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 65536);
    
    ASSERT_EQ(CGI::SETUP_ERROR_INVALID_PORT, result);
}

TEST(setup_path_traversal_script) {
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, "../../../etc/passwd", "/usr/bin/python3", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_ERROR_PATH_TRAVERSAL, result);
}

TEST(setup_path_traversal_interpreter) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "../bin/python3", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_ERROR_PATH_TRAVERSAL, result);
}

TEST(setup_script_not_found) {
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, "/nonexistent/script.py", "/usr/bin/python3", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_ERROR_SCRIPT_NOT_FOUND, result);
}

TEST(setup_interpreter_not_found) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    if (access(scriptPath.c_str(), R_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED: Script not available" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/nonexistent/interpreter", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_ERROR_INTERPRETER_NOT_FOUND, result);
}

TEST(setup_valid_inputs) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED: Script not available" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/hello.py?test=1");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    
    ASSERT_EQ(CGI::SETUP_OK, result);
    ASSERT_TRUE(cgi.getError().empty());
}

TEST(setup_result_messages) {
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_OK), "successful");
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_ERROR_EMPTY_SCRIPT_PATH), "empty");
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_ERROR_INVALID_PORT), "port");
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_ERROR_PATH_TRAVERSAL), "traversal");
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_ERROR_SCRIPT_NOT_FOUND), "Script");
    ASSERT_CONTAINS(CGI::getSetupResultMessage(CGI::SETUP_ERROR_INTERPRETER_NOT_FOUND), "Interpreter");
}

// ============================================================================
// Execute Tests
// ============================================================================

TEST(execute_without_setup) {
    CGI cgi;
    int readFd, writeFd;
    
    pid_t pid = cgi.execute(readFd, writeFd);
    
    ASSERT_EQ(-1, pid);
    ASSERT_CONTAINS(cgi.getError(), "setup");
}

TEST(execute_python_script) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0 || access("/usr/bin/python3", X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/hello.py?name=test");
    
    CGI cgi;
    CGI::SetupResult setupResult = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    ASSERT_EQ(CGI::SETUP_OK, setupResult);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    
    int exitStatus = waitForChild(pid);
    ASSERT_EQ(0, exitStatus);
    
    std::map<std::string, std::string> headers;
    std::string body;
    bool parsed = CGI::parseCGIOutput(output, headers, body);
    
    ASSERT_TRUE(parsed);
    ASSERT_EQ(std::string("text/html"), headers["Content-Type"]);
    ASSERT_CONTAINS(body, "Hello from Python CGI!");
}

TEST(execute_shell_script) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.sh";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/hello.sh?foo=bar");
    
    CGI cgi;
    CGI::SetupResult setupResult = cgi.setup(request, scriptPath, "/bin/bash", "localhost", 8080);
    ASSERT_EQ(CGI::SETUP_OK, setupResult);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, "Hello from Shell CGI!");
    ASSERT_CONTAINS(output, "REQUEST_METHOD=GET");
}

TEST(execute_environment_variables) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/echo_env.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0 || access("/usr/bin/python3", X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::POST);
    request.setUri("/cgi-bin/echo_env.py?query=value");
    request.setHeader("Content-Type", "application/json");
    request.setHeader("X-Custom-Header", "custom-value");
    request.setBody("{\"key\": \"value\"}");
    
    CGI cgi;
    CGI::SetupResult setupResult = cgi.setup(request, scriptPath, "/usr/bin/python3", "testserver", 9090);
    ASSERT_EQ(CGI::SETUP_OK, setupResult);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, "REQUEST_METHOD=POST");
    ASSERT_CONTAINS(output, "QUERY_STRING=query=value");
    ASSERT_CONTAINS(output, "CONTENT_TYPE=application/json");
    ASSERT_CONTAINS(output, "SERVER_NAME=testserver");
    ASSERT_CONTAINS(output, "SERVER_PORT=9090");
    ASSERT_CONTAINS(output, "HTTP_X_CUSTOM_HEADER=custom-value");
}

TEST(execute_post_with_body) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/post_echo.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0 || access("/usr/bin/python3", X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    std::string postBody = "This is the POST body content";
    
    libhttp::Request request;
    request.setMethod(libhttp::POST);
    request.setUri("/cgi-bin/post_echo.py");
    request.setHeader("Content-Type", "text/plain");
    request.setBody(postBody);
    
    std::ostringstream ss;
    ss << postBody.length();
    request.setHeader("Content-Length", ss.str());
    
    CGI cgi;
    cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    // Write POST body to CGI script, checking for errors
    ssize_t bytesWritten = write(writeFd, postBody.c_str(), postBody.length());
    ASSERT_EQ(static_cast<ssize_t>(postBody.length()), bytesWritten);
    close(writeFd);
    
    std::string output = readFromFd(readFd);
    close(readFd);
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, postBody);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(edge_case_empty_query_string) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/echo_env.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/echo_env.py");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    ASSERT_EQ(CGI::SETUP_OK, result);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, "QUERY_STRING=\n");
}

TEST(edge_case_uri_with_only_question_mark) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/echo_env.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/echo_env.py?");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    ASSERT_EQ(CGI::SETUP_OK, result);
}

TEST(edge_case_special_chars_in_query) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/echo_env.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/cgi-bin/echo_env.py?key=value%20with%20spaces&special=<>&test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 8080);
    ASSERT_EQ(CGI::SETUP_OK, result);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, "QUERY_STRING=key=value%20with%20spaces&special=<>&test");
}

TEST(edge_case_max_port) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/hello.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "localhost", 65535);
    ASSERT_EQ(CGI::SETUP_OK, result);
}

TEST(edge_case_empty_server_name) {
    std::string cwd = getCwd();
    std::string scriptPath = cwd + "/tests/cgi-bin/echo_env.py";
    
    if (access(scriptPath.c_str(), X_OK) != 0) {
        std::cerr << COLOR_YELLOW << "  SKIPPED" << COLOR_RESET << std::endl;
        return;
    }
    
    libhttp::Request request;
    request.setMethod(libhttp::GET);
    request.setUri("/test");
    
    CGI cgi;
    CGI::SetupResult result = cgi.setup(request, scriptPath, "/usr/bin/python3", "", 8080);
    ASSERT_EQ(CGI::SETUP_OK, result);
    
    int readFd, writeFd;
    pid_t pid = cgi.execute(readFd, writeFd);
    ASSERT_NE(-1, pid);
    
    close(writeFd);
    std::string output = readFromFd(readFd);
    close(readFd);
    waitForChild(pid);
    
    ASSERT_CONTAINS(output, "SERVER_NAME=\n");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "\n" << COLOR_CYAN << "=== CGI Test Suite ===" << COLOR_RESET << "\n";
    std::cout << "Testing: DRY, SOLID, KISS principles + Edge Cases\n\n";
    
    // Utils tests (libutils namespace - DRY verification)
    std::cout << COLOR_YELLOW << "--- Utils Tests (libutils) ---" << COLOR_RESET << "\n";
    RUN_TEST(utils_trim);
    RUN_TEST(utils_trimLeft);
    RUN_TEST(utils_trimRight);
    RUN_TEST(utils_toUpper);
    RUN_TEST(utils_toLower);
    RUN_TEST(utils_getDirectory);
    RUN_TEST(utils_getFilename);
    RUN_TEST(utils_getExtension);
    RUN_TEST(utils_hasExtension);
    RUN_TEST(utils_isValidPort);
    RUN_TEST(utils_containsPathTraversal);
    RUN_TEST(utils_sanitizeForEnv);
    RUN_TEST(utils_intToString);
    RUN_TEST(utils_sizeToString);
    RUN_TEST(utils_stringToInt);
    
    // libhttp Utils tests
    std::cout << "\n" << COLOR_YELLOW << "--- libhttp Utils Tests ---" << COLOR_RESET << "\n";
    RUN_TEST(utils_methodToString);
    
    // isCGIScript tests
    std::cout << "\n" << COLOR_YELLOW << "--- isCGIScript Tests ---" << COLOR_RESET << "\n";
    RUN_TEST(isCGIScript_py_extension);
    RUN_TEST(isCGIScript_cgi_extension);
    RUN_TEST(isCGIScript_wrong_extension);
    RUN_TEST(isCGIScript_no_extension);
    RUN_TEST(isCGIScript_empty_path);
    RUN_TEST(isCGIScript_empty_extension);
    
    // parseCGIOutput tests
    std::cout << "\n" << COLOR_YELLOW << "--- parseCGIOutput Tests ---" << COLOR_RESET << "\n";
    RUN_TEST(parseCGIOutput_simple_headers);
    RUN_TEST(parseCGIOutput_multiple_headers);
    RUN_TEST(parseCGIOutput_lf_line_endings);
    RUN_TEST(parseCGIOutput_no_body);
    RUN_TEST(parseCGIOutput_no_headers);
    RUN_TEST(parseCGIOutput_empty_output);
    RUN_TEST(parseCGIOutput_header_with_spaces);
    RUN_TEST(parseCGIOutput_binary_body);
    
    // Setup validation tests (Security & Edge Cases)
    std::cout << "\n" << COLOR_YELLOW << "--- Setup Validation Tests (Security) ---" << COLOR_RESET << "\n";
    RUN_TEST(setup_empty_script_path);
    RUN_TEST(setup_invalid_port_zero);
    RUN_TEST(setup_invalid_port_negative);
    RUN_TEST(setup_invalid_port_too_high);
    RUN_TEST(setup_path_traversal_script);
    RUN_TEST(setup_path_traversal_interpreter);
    RUN_TEST(setup_script_not_found);
    RUN_TEST(setup_interpreter_not_found);
    RUN_TEST(setup_valid_inputs);
    RUN_TEST(setup_result_messages);
    
    // Execute tests
    std::cout << "\n" << COLOR_YELLOW << "--- Execute Tests ---" << COLOR_RESET << "\n";
    RUN_TEST(execute_without_setup);
    RUN_TEST(execute_python_script);
    RUN_TEST(execute_shell_script);
    RUN_TEST(execute_environment_variables);
    RUN_TEST(execute_post_with_body);
    
    // Edge case tests
    std::cout << "\n" << COLOR_YELLOW << "--- Edge Case Tests ---" << COLOR_RESET << "\n";
    RUN_TEST(edge_case_empty_query_string);
    RUN_TEST(edge_case_uri_with_only_question_mark);
    RUN_TEST(edge_case_special_chars_in_query);
    RUN_TEST(edge_case_max_port);
    RUN_TEST(edge_case_empty_server_name);
    
    // Summary
    std::cout << "\n" << COLOR_CYAN << "=== Test Summary ===" << COLOR_RESET << "\n";
    std::cout << "Total:  " << g_testsRun << "\n";
    std::cout << COLOR_GREEN << "Passed: " << g_testsPassed << COLOR_RESET << "\n";
    if (g_testsFailed > 0) {
        std::cout << COLOR_RED << "Failed: " << g_testsFailed << COLOR_RESET << "\n";
    }
    
    return g_testsFailed > 0 ? 1 : 0;
}
