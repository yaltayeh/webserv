#include "Config.hpp"
#include "Parser.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DenyDirective.hpp"
#include "LimitExceptDirective.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

using namespace Directive;

void testDenyDirective() {
    std::cout << "\n=== Testing Deny Directive ===" << std::endl;
    
    // Test deny with IP address
    std::vector<std::string> args1;
    args1.push_back("deny");
    args1.push_back("192.168.1.1");
    DenyDirective* deny1 = DenyDirective::initDenyDirective(args1);
    assert(deny1->address == "192.168.1.1");
    std::cout << "✓ Deny IP: " << deny1->address << std::endl;
    delete deny1;
    
    // Test deny with CIDR
    std::vector<std::string> args2;
    args2.push_back("deny");
    args2.push_back("10.0.0.0/8");
    DenyDirective* deny2 = DenyDirective::initDenyDirective(args2);
    assert(deny2->address == "10.0.0.0/8");
    std::cout << "✓ Deny CIDR: " << deny2->address << std::endl;
    delete deny2;
    
    // Test deny all
    std::vector<std::string> args3;
    args3.push_back("deny");
    args3.push_back("all");
    DenyDirective* deny3 = DenyDirective::initDenyDirective(args3);
    assert(deny3->address == "all");
    std::cout << "✓ Deny all: " << deny3->address << std::endl;
    delete deny3;
    
    // Test invalid argument count
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("deny");
        DenyDirective::initDenyDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Invalid args correctly rejected: " << e.what() << std::endl;
    }
}

void testLimitExceptDirective() {
    std::cout << "\n=== Testing LimitExcept Directive ===" << std::endl;
    
    // Test single method
    std::vector<std::string> args1;
    args1.push_back("limit_except");
    args1.push_back("GET");
    LimitExceptDirective* limit1 = LimitExceptDirective::initLimitExceptDirective(args1);
    assert(limit1->methods.size() == 1);
    assert(limit1->methods[0] == "GET");
    std::cout << "✓ Single method: GET" << std::endl;
    delete limit1;
    
    // Test multiple methods
    std::vector<std::string> args2;
    args2.push_back("limit_except");
    args2.push_back("GET");
    args2.push_back("POST");
    args2.push_back("DELETE");
    LimitExceptDirective* limit2 = LimitExceptDirective::initLimitExceptDirective(args2);
    assert(limit2->methods.size() == 3);
    assert(limit2->methods[0] == "GET");
    assert(limit2->methods[1] == "POST");
    assert(limit2->methods[2] == "DELETE");
    std::cout << "✓ Multiple methods: GET, POST, DELETE" << std::endl;
    delete limit2;
    
    // Test invalid method
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("limit_except");
        argsInvalid.push_back("INVALID");
        LimitExceptDirective::initLimitExceptDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Invalid method correctly rejected: " << e.what() << std::endl;
    }
    
    // Test no methods
    try {
        std::vector<std::string> argsEmpty;
        argsEmpty.push_back("limit_except");
        LimitExceptDirective::initLimitExceptDirective(argsEmpty);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Empty methods correctly rejected: " << e.what() << std::endl;
    }
}

void testAccessControlConfig() {
    std::cout << "\n=== Testing Access Control Configuration ===" << std::endl;
    
    // Create test config file
    std::ofstream configFile("tests/access_control.conf");
    configFile << "http {\n";
    configFile << "    deny 192.168.1.100;\n";
    configFile << "    \n";
    configFile << "    server {\n";
    configFile << "        listen 80;\n";
    configFile << "        server_name example.com;\n";
    configFile << "        deny 10.0.0.0/8;\n";
    configFile << "        \n";
    configFile << "        location /public/ {\n";
    configFile << "            root /var/www/public;\n";
    configFile << "        }\n";
    configFile << "        \n";
    configFile << "        location /api/ {\n";
    configFile << "            root /var/www/api;\n";
    configFile << "            deny all;\n";
    configFile << "            limit_except GET POST {\n";
    configFile << "                deny all;\n";
    configFile << "            }\n";
    configFile << "        }\n";
    configFile << "        \n";
    configFile << "        location /admin/ {\n";
    configFile << "            root /var/www/admin;\n";
    configFile << "            deny 192.168.1.0/24;\n";
    configFile << "            limit_except GET {\n";
    configFile << "            }\n";
    configFile << "        }\n";
    configFile << "    }\n";
    configFile << "}\n";
    configFile.close();
    
    // Parse config
    Parser parser = Parser::openConfigFile("tests/access_control.conf");
    while (parser.getNextDirectives()) {
        IDirective::parseDirective(global, parser.copyDirective());
    }
    
    // Get http directive
    const HttpDirective* http = global->getFirstChild<HttpDirective>();
    std::cout << "✓ HTTP directive parsed" << std::endl;
    
    // Check http-level deny
    std::vector<DenyDirective*> httpDenies = http->getChildren<DenyDirective>();
    assert(httpDenies.size() == 1);
    assert(httpDenies[0]->address == "192.168.1.100");
    std::cout << "✓ HTTP-level deny: " << httpDenies[0]->address << std::endl;
    
    // Get server
    std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
    assert(servers.size() == 1);
    const ServerDirective* server = servers[0];
    std::cout << "✓ Server directive parsed" << std::endl;
    
    // Check server-level deny
    std::vector<DenyDirective*> serverDenies = server->getChildren<DenyDirective>();
    assert(serverDenies.size() == 1);
    assert(serverDenies[0]->address == "10.0.0.0/8");
    std::cout << "✓ Server-level deny: " << serverDenies[0]->address << std::endl;
    
    // Get locations
    std::vector<LocationDirective*> locations = server->getChildren<LocationDirective>();
    assert(locations.size() == 3);
    std::cout << "✓ Found " << locations.size() << " locations" << std::endl;
    
    // Check /public/ location (no deny or limit_except)
    const LocationDirective* publicLoc = locations[0];
    assert(publicLoc->uri == "/public/");
    std::vector<DenyDirective*> publicDenies = publicLoc->getChildren<DenyDirective>();
    assert(publicDenies.size() == 0);
    std::vector<LimitExceptDirective*> publicLimits = publicLoc->getChildren<LimitExceptDirective>();
    assert(publicLimits.size() == 0);
    std::cout << "✓ /public/ location has no restrictions" << std::endl;
    
    // Check /api/ location
    const LocationDirective* apiLoc = locations[1];
    assert(apiLoc->uri == "/api/");
    std::vector<DenyDirective*> apiDenies = apiLoc->getChildren<DenyDirective>();
    assert(apiDenies.size() == 1);
    assert(apiDenies[0]->address == "all");
    std::cout << "✓ /api/ deny: " << apiDenies[0]->address << std::endl;
    
    std::vector<LimitExceptDirective*> apiLimits = apiLoc->getChildren<LimitExceptDirective>();
    assert(apiLimits.size() == 1);
    assert(apiLimits[0]->methods.size() == 2);
    assert(apiLimits[0]->methods[0] == "GET");
    assert(apiLimits[0]->methods[1] == "POST");
    std::cout << "✓ /api/ limit_except: GET, POST allowed" << std::endl;
    
    // Check nested deny inside limit_except
    std::vector<DenyDirective*> limitDenies = apiLimits[0]->getChildren<DenyDirective>();
    assert(limitDenies.size() == 1);
    assert(limitDenies[0]->address == "all");
    std::cout << "✓ /api/ limit_except contains deny all" << std::endl;
    
    // Check /admin/ location
    const LocationDirective* adminLoc = locations[2];
    assert(adminLoc->uri == "/admin/");
    std::vector<DenyDirective*> adminDenies = adminLoc->getChildren<DenyDirective>();
    assert(adminDenies.size() == 1);
    assert(adminDenies[0]->address == "192.168.1.0/24");
    std::cout << "✓ /admin/ deny: " << adminDenies[0]->address << std::endl;
    
    std::vector<LimitExceptDirective*> adminLimits = adminLoc->getChildren<LimitExceptDirective>();
    assert(adminLimits.size() == 1);
    assert(adminLimits[0]->methods.size() == 1);
    assert(adminLimits[0]->methods[0] == "GET");
    std::cout << "✓ /admin/ limit_except: GET allowed" << std::endl;
    
    std::cout << "\n✓ All access control tests passed!" << std::endl;
}

int main() {
    try {
        g_log_level = LOG_LEVEL_WARN;  // Reduce log noise
        
        testDenyDirective();
        testLimitExceptDirective();
        testAccessControlConfig();
        
        std::cout << "\n==================================" << std::endl;
        std::cout << "All tests passed successfully!" << std::endl;
        std::cout << "==================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
