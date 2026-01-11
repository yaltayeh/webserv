#include "Config.hpp"
#include "Parser.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "AliasDirective.hpp"
#include "RootDirective.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace Directive;

void testAliasDirective() {
    std::cout << "\n=== Testing Alias Directive ===" << std::endl;
    
    // Test alias with path
    std::vector<std::string> args1;
    args1.push_back("alias");
    args1.push_back("/data/pictures/");
    AliasDirective* alias1 = AliasDirective::initAliasDirective(args1);
    assert(alias1->path == "/data/pictures/");
    std::cout << "✓ Alias path: " << alias1->path << std::endl;
    delete alias1;
    
    // Test invalid argument count
    try {
        std::vector<std::string> argsInvalid;
        argsInvalid.push_back("alias");
        AliasDirective::initAliasDirective(argsInvalid);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Invalid args correctly rejected: " << e.what() << std::endl;
    }
    
    // Test empty path
    try {
        std::vector<std::string> argsEmpty;
        argsEmpty.push_back("alias");
        argsEmpty.push_back("");
        AliasDirective::initAliasDirective(argsEmpty);
        assert(false && "Should have thrown ArgumentException");
    } catch (const ArgumentException& e) {
        std::cout << "✓ Empty path correctly rejected: " << e.what() << std::endl;
    }
}

void testAliasConfig() {
    std::cout << "\n=== Testing Alias in Configuration ===" << std::endl;
    
    // Create test config file
    std::ofstream configFile("tests/alias_test.conf");
    configFile << "http {\n";
    configFile << "    server {\n";
    configFile << "        listen 80;\n";
    configFile << "        server_name example.com;\n";
    configFile << "        \n";
    configFile << "        # Regular root location\n";
    configFile << "        location / {\n";
    configFile << "            root /var/www/html;\n";
    configFile << "        }\n";
    configFile << "        \n";
    configFile << "        # Alias location\n";
    configFile << "        location /images/ {\n";
    configFile << "            alias /data/pictures/;\n";
    configFile << "        }\n";
    configFile << "        \n";
    configFile << "        # Another alias\n";
    configFile << "        location /static/ {\n";
    configFile << "            alias /usr/share/nginx/static/;\n";
    configFile << "        }\n";
    configFile << "    }\n";
    configFile << "}\n";
    configFile.close();
    
    // Parse config
    Parser parser = Parser::openConfigFile("tests/alias_test.conf");
    while (parser.getNextDirectives()) {
        IDirective::parseDirective(global, parser.copyDirective());
    }
    
    // Get http directive
    const HttpDirective* http = global->getFirstChild<HttpDirective>();
    std::cout << "✓ HTTP directive parsed" << std::endl;
    
    // Get server
    std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
    assert(servers.size() == 1);
    const ServerDirective* server = servers[0];
    std::cout << "✓ Server directive parsed" << std::endl;
    
    // Get locations
    std::vector<LocationDirective*> locations = server->getChildren<LocationDirective>();
    assert(locations.size() == 3);
    std::cout << "✓ Found " << locations.size() << " locations" << std::endl;
    
    // Check first location with root
    const LocationDirective* rootLoc = locations[0];
    assert(rootLoc->uri == "/");
    try {
        const RootDirective* root = rootLoc->getFirstChild<RootDirective>();
        assert(root->path == "/var/www/html");
        std::cout << "✓ Location / uses root: " << root->path << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "Root directive should exist");
    }
    
    // Verify no alias in root location
    std::vector<AliasDirective*> rootAliases = rootLoc->getChildren<AliasDirective>();
    assert(rootAliases.size() == 0);
    std::cout << "✓ Location / has no alias (uses root)" << std::endl;
    
    // Check second location with alias
    const LocationDirective* imagesLoc = locations[1];
    assert(imagesLoc->uri == "/images/");
    try {
        const AliasDirective* alias = imagesLoc->getFirstChild<AliasDirective>();
        assert(alias->path == "/data/pictures/");
        std::cout << "✓ Location /images/ uses alias: " << alias->path << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "Alias directive should exist");
    }
    
    // Verify no root in alias location
    std::vector<RootDirective*> imageRoots = imagesLoc->getChildren<RootDirective>();
    assert(imageRoots.size() == 0);
    std::cout << "✓ Location /images/ has no root (uses alias)" << std::endl;
    
    // Check third location with alias
    const LocationDirective* staticLoc = locations[2];
    assert(staticLoc->uri == "/static/");
    try {
        const AliasDirective* alias = staticLoc->getFirstChild<AliasDirective>();
        assert(alias->path == "/usr/share/nginx/static/");
        std::cout << "✓ Location /static/ uses alias: " << alias->path << std::endl;
    } catch (const DirectiveNotFound&) {
        assert(false && "Alias directive should exist");
    }
    
    std::cout << "\n✓ All alias tests passed!" << std::endl;
}

int main() {
    try {
        g_log_level = LOG_LEVEL_WARN;  // Reduce log noise
        
        testAliasDirective();
        testAliasConfig();
        
        std::cout << "\n==================================" << std::endl;
        std::cout << "All tests passed successfully!" << std::endl;
        std::cout << "==================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
