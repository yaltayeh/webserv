#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "../dependencies/libhttp/include/Parser.hpp"
#include "../dependencies/libhttp/include/Request.hpp"


// ❯ rm -rf test_temp && c++ -Wall -Wextra -Werror -std=c++98 -Idependencies/libhttp/include -Idependencies/libutils/include test_libhttp.cpp dependencies/libhttp/src/*.cpp dependencies/libutils/src/*.cpp -o

void printRequest(const libhttp::Request& req) {
    std::cout << "Method: " << req.getMethod() << std::endl;
    std::cout << "URI: " << req.getUri() << std::endl;
    std::cout << "Body length: " << req.getBody().length() << std::endl;
    std::cout << "Has temp file: " << (req.hasBodyTempFile() ? "YES" : "NO") << std::endl;
    if (req.hasBodyTempFile()) {
        std::cout << "Temp file path: " << req.getBodyTempFile() << std::endl;
    }
    std::cout << std::endl;
}

void testSmallBody() {
    std::cout << "=== Test 1: Small body (should stay in memory) ===" << std::endl;
    
    // Create parser with 1KB limit
    libhttp::Parser parser(1024, "test_temp");
    
    std::string request = 
        "POST /upload HTTP/1.0\r\n"
        "Content-Length: 13\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, World!";
    
    int result = parser.parse(request.c_str(), request.length());
    
    std::cout << "Parse result: " << result << " (0=complete, 1=incomplete, -1=error)" << std::endl;
    std::cout << "Is complete: " << (parser.isComplete() ? "YES" : "NO") << std::endl;
    std::cout << "Is error: " << (parser.isError() ? "YES" : "NO") << std::endl;
    
    if (parser.isComplete()) {
        libhttp::Request req = parser.getRequest();
        printRequest(req);
        std::cout << "Body content: " << req.getBody() << std::endl;
    }
    
    std::cout << std::endl;
}

void testLargeBody() {
    std::cout << "=== Test 2: Large body (should go to temp file) ===" << std::endl;
    
    std::string tempFilePath;
    
    {
        // Create parser with 100 byte limit
        libhttp::Parser parser(100, "test_temp");
        
        // Create a body larger than 100 bytes
        std::string largeBody(200, 'X');
        
        std::string request = 
            "POST /upload HTTP/1.0\r\n"
            "Content-Length: 200\r\n"
            "Content-Type: application/octet-stream\r\n"
            "\r\n" + largeBody;
        
        int result = parser.parse(request.c_str(), request.length());
        
        std::cout << "Parse result: " << result << " (0=complete, 1=incomplete, -1=error)" << std::endl;
        std::cout << "Is complete: " << (parser.isComplete() ? "YES" : "NO") << std::endl;
        std::cout << "Is error: " << (parser.isError() ? "YES" : "NO") << std::endl;
        
        if (parser.isComplete()) {
            libhttp::Request req = parser.getRequest();
            printRequest(req);
            std::cout << "In-memory body length: " << req.getBody().length() << std::endl;
            
            if (req.hasBodyTempFile()) {
                tempFilePath = req.getBodyTempFile();
            }
        }
    } // Parser destructor runs here, closing the file
    
    // Now check the temp file after parser is destroyed
    if (!tempFilePath.empty()) {
        std::ifstream tempFile(tempFilePath.c_str(), std::ios::binary);
        if (tempFile.is_open()) {
            tempFile.seekg(0, std::ios::end);
            size_t fileSize = tempFile.tellg();
            std::cout << "Temp file size after parser destroyed: " << fileSize << " bytes" << std::endl;
            
            // Read first 20 chars to verify content
            tempFile.seekg(0, std::ios::beg);
            char buffer[21] = {0};
            tempFile.read(buffer, 20);
            std::cout << "First 20 bytes: " << buffer << std::endl;
            tempFile.close();
        }
    }
    
    std::cout << std::endl;
}

void testChunkedReading() {
    std::cout << "=== Test 3: Chunked reading with temp file ===" << std::endl;
    
    // Create parser with 50 byte limit
    libhttp::Parser parser(50, "test_temp");
    
    // Send request in multiple chunks
    std::string chunk1 = "POST /upload HTTP/1.0\r\n";
    std::string chunk2 = "Content-Length: 100\r\n";
    std::string chunk3 = "Content-Type: text/plain\r\n\r\n";
    std::string bodyPart1(50, 'A');
    std::string bodyPart2(50, 'B');
    
    int result;
    
    result = parser.parse(chunk1.c_str(), chunk1.length());
    std::cout << "After chunk 1: result=" << result << std::endl;
    
    result = parser.parse(chunk2.c_str(), chunk2.length());
    std::cout << "After chunk 2: result=" << result << std::endl;
    
    result = parser.parse(chunk3.c_str(), chunk3.length());
    std::cout << "After chunk 3 (headers done): result=" << result << std::endl;
    
    result = parser.parse(bodyPart1.c_str(), bodyPart1.length());
    std::cout << "After body part 1: result=" << result << std::endl;
    
    result = parser.parse(bodyPart2.c_str(), bodyPart2.length());
    std::cout << "After body part 2: result=" << result << std::endl;
    
    std::cout << "Is complete: " << (parser.isComplete() ? "YES" : "NO") << std::endl;
    
    if (parser.isComplete()) {
        libhttp::Request req = parser.getRequest();
        printRequest(req);
    }
    
    std::cout << std::endl;
}

void testBodyExceedsLimit() {
    std::cout << "=== Test 4: Body exceeds max limit (should error) ===" << std::endl;
    
    // Create parser with 50 byte limit
    libhttp::Parser parser(50, "test_temp");
    
    std::string request = 
        "POST /upload HTTP/1.0\r\n"
        "Content-Length: 100\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n";
    
    int result = parser.parse(request.c_str(), request.length());
    
    std::cout << "Parse result: " << result << " (0=complete, 1=incomplete, -1=error)" << std::endl;
    std::cout << "Is error: " << (parser.isError() ? "YES" : "NO") << std::endl;
    
    std::cout << std::endl;
}

void testDefaultParser() {
    std::cout << "=== Test 5: Default parser (1MB limit) ===" << std::endl;
    
    libhttp::Parser parser;
    
    std::string request = 
        "GET /index.html HTTP/1.0\r\n"
        "Host: localhost\r\n"
        "\r\n";
    
    int result = parser.parse(request.c_str(), request.length());
    
    std::cout << "Parse result: " << result << std::endl;
    std::cout << "Is complete: " << (parser.isComplete() ? "YES" : "NO") << std::endl;
    
    if (parser.isComplete()) {
        libhttp::Request req = parser.getRequest();
        printRequest(req);
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "libhttp Parser Test Suite" << std::endl;
    std::cout << "=========================" << std::endl << std::endl;
    
    testSmallBody();
    testLargeBody();
    testChunkedReading();
    testBodyExceedsLimit();
    testDefaultParser();
    
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}
