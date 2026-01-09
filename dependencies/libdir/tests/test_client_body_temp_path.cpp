#include "Config.hpp"
#include "Parser.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "ClientBodyTempPathDirective.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace Config;

void testClientBodyTempPathDirective() {
    std::cout << "\n=== Testing ClientBodyTempPath Directive ===" << std::endl;
    
    // Test with path only
    std::vector<std::string> args1;
    args1.push_back("client_body_temp_path");
    args1.push_back("/var/tmp/client_body");
    ClientBodyTempPathDirective* temp1 = ClientBodyTempPathDirective::initClientBodyTempPathDirective(args1);
    assert(temp1->path == "/var/tmp/client_body");
    assert(temp1->level1 == 0);
    assert(temp1->level2 == 0);
    assert(temp1->level3 == 0);
    std::cout << "✓ Path only: " << temp1->path << std::endl;
    delete temp1;
    
    // Test with path and level1
    std::vector<std::string> args2;
    args2.push_back("client_body_temp_path");
    args2.push_back("/tmp/bodies");
    args2.push_back("1");
    ClientBodyTempPathDirective* temp2 = ClientBodyTempPathDirective::initClientBodyTempPathDirective(args2);
    assert(temp2->path == "/tmp/bodies");
    assert(temp2->level1 == 1);
    assert(temp2->level2 == 0);
    assert(temp2->level3 == 0);
    std::cout << "✓ Path + level1: " << temp2->path << " " << temp2->level1 << std::endl;
    delete temp2;
    
    // Test with path and all levels
    std::vector<std::string> args3;
    args3.push_back("client_body_temp_path");
    args3.push_back("/var/tmp/nginx");
    args3.push_back("2");
    args3.push_back("3");
    args3.push_back("4");
    ClientBodyTempPathDirective* temp3 = ClientBodyTempPathDirective::initClientBodyTempPathDirective(args3);
    assert(temp3->path == "/var/tmp/nginx");
    assert(temp3->level1 == 2);
    assert(temp3->level2 == 3);
    assert(temp3->level3 == 4);
    std::cout << "✓ Path + all levels: " << temp3->path << " " << temp3->level1 
              << " " << temp3->level2 << " " << temp3->level3 << std::endl;
    delete temp3;
    
    // Test invalid level (too large)
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("client_body_temp_path");
        argsInvalid.push_back("/tmp");
        argsInvalid.push_back("11");  // > 10
        ClientBodyTempPathDirective::initClientBodyTempPathDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Invalid level correctly rejected: " << e.what() << std::endl;
    }
    
    // Test invalid level (negative)
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("client_body_temp_path");
        argsInvalid.push_back("/tmp");
        argsInvalid.push_back("-1");
        ClientBodyTempPathDirective::initClientBodyTempPathDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Negative level correctly rejected: " << e.what() << std::endl;
    }
    
    // Test invalid level (not a number)
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("client_body_temp_path");
        argsInvalid.push_back("/tmp");
        argsInvalid.push_back("abc");
        ClientBodyTempPathDirective::initClientBodyTempPathDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Non-numeric level correctly rejected: " << e.what() << std::endl;
    }
    
    // Test no arguments
    try {
        std::vector<std::string> argsEmpty;
        argsEmpty.push_back("client_body_temp_path");
        ClientBodyTempPathDirective::initClientBodyTempPathDirective(argsEmpty);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Empty args correctly rejected: " << e.what() << std::endl;
    }
}

void testClientBodyTempPathConfig() {
    std::cout << "\n=== Testing ClientBodyTempPath Configuration ===" << std::endl;
    
    // Create test config file
    std::ofstream configFile("tests/client_body_temp.conf");
    configFile << "http {\n";
    configFile << "    client_body_temp_path /var/tmp/http_temp;\n";
    configFile << "    \n";
    configFile << "    server {\n";
    configFile << "        listen 80;\n";
    configFile << "        client_body_temp_path /var/tmp/server_temp 1;\n";
    configFile << "        \n";
    configFile << "        location /upload/ {\n";
    configFile << "            client_body_temp_path /var/tmp/upload_temp 1 2 3;\n";
    configFile << "        }\n";
    configFile << "        \n";
    configFile << "        location /api/ {\n";
    configFile << "            # Uses server-level temp path\n";
    configFile << "        }\n";
    configFile << "    }\n";
    configFile << "}\n";
    configFile.close();
    
    // Parse config
    Parser parser = Parser::openConfigFile("tests/client_body_temp.conf");
    while (parser.getNextDirectives()) {
        IDirective::parseDirective(global, parser.copyDirective());
    }
    
    // Get http directive
    const HttpDirective* http = global->getFirstChild<HttpDirective>();
    std::cout << "✓ HTTP directive parsed" << std::endl;
    
    // Check http-level client_body_temp_path
    try {
        const ClientBodyTempPathDirective* httpTemp = 
            http->getFirstChild<ClientBodyTempPathDirective>();
        assert(httpTemp->path == "/var/tmp/http_temp");
        assert(httpTemp->level1 == 0);
        std::cout << "✓ HTTP-level temp path: " << httpTemp->path << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "HTTP temp path should exist");
    }
    
    // Get server
    std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
    assert(servers.size() == 1);
    const ServerDirective* server = servers[0];
    std::cout << "✓ Server directive parsed" << std::endl;
    
    // Check server-level client_body_temp_path
    try {
        const ClientBodyTempPathDirective* serverTemp = 
            server->getFirstChild<ClientBodyTempPathDirective>();
        assert(serverTemp->path == "/var/tmp/server_temp");
        assert(serverTemp->level1 == 1);
        assert(serverTemp->level2 == 0);
        assert(serverTemp->level3 == 0);
        std::cout << "✓ Server-level temp path: " << serverTemp->path 
                  << " (level1=" << serverTemp->level1 << ")" << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "Server temp path should exist");
    }
    
    // Get locations
    std::vector<LocationDirective*> locations = server->getChildren<LocationDirective>();
    assert(locations.size() == 2);
    std::cout << "✓ Found " << locations.size() << " locations" << std::endl;
    
    // Check /upload/ location
    const LocationDirective* uploadLoc = locations[0];
    assert(uploadLoc->uri == "/upload/");
    try {
        const ClientBodyTempPathDirective* uploadTemp = 
            uploadLoc->getFirstChild<ClientBodyTempPathDirective>();
        assert(uploadTemp->path == "/var/tmp/upload_temp");
        assert(uploadTemp->level1 == 1);
        assert(uploadTemp->level2 == 2);
        assert(uploadTemp->level3 == 3);
        std::cout << "✓ /upload/ temp path: " << uploadTemp->path 
                  << " (levels: " << uploadTemp->level1 << "," 
                  << uploadTemp->level2 << "," << uploadTemp->level3 << ")" << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "Upload location temp path should exist");
    }
    
    // Check /api/ location (should not have client_body_temp_path)
    const LocationDirective* apiLoc = locations[1];
    assert(apiLoc->uri == "/api/");
    try {
        apiLoc->getFirstChild<ClientBodyTempPathDirective>();
        assert(false && "API location should not have temp path");
    } catch (const DirectiveNotFound&) {
        std::cout << "✓ /api/ location has no temp path (inherits from server)" << std::endl;
    }
    
    std::cout << "\n✓ All client_body_temp_path tests passed!" << std::endl;
}

int main() {
    try {
        g_log_level = LOG_LEVEL_WARN;  // Reduce log noise
        
        testClientBodyTempPathDirective();
        testClientBodyTempPathConfig();
        
        std::cout << "\n==================================" << std::endl;
        std::cout << "All tests passed successfully!" << std::endl;
        std::cout << "==================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
