#include "Config.hpp"
#include "Parser.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "ProxyRequestBufferingDirective.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace Config;

int main() {
    try {
        g_log_level = LOG_LEVEL_WARN;
        
        std::cout << "=== Testing ProxyRequestBuffering Directive ===" << std::endl;
        
        // Test on
        std::vector<std::string> args1;
        args1.push_back("proxy_request_buffering");
        args1.push_back("on");
        ProxyRequestBufferingDirective* buff1 = ProxyRequestBufferingDirective::initProxyRequestBufferingDirective(args1);
        assert(buff1->enabled == true);
        std::cout << "✓ proxy_request_buffering on: enabled=" << (buff1->enabled ? "true" : "false") << std::endl;
        delete buff1;
        
        // Test off
        std::vector<std::string> args2;
        args2.push_back("proxy_request_buffering");
        args2.push_back("off");
        ProxyRequestBufferingDirective* buff2 = ProxyRequestBufferingDirective::initProxyRequestBufferingDirective(args2);
        assert(buff2->enabled == false);
        std::cout << "✓ proxy_request_buffering off: enabled=" << (buff2->enabled ? "true" : "false") << std::endl;
        delete buff2;
        
        // Test invalid value
        try {
            std::vector<std::string> argsInvalid;
            argsInvalid.push_back("proxy_request_buffering");
            argsInvalid.push_back("maybe");
            ProxyRequestBufferingDirective::initProxyRequestBufferingDirective(argsInvalid);
            assert(false && "Should have thrown ArgumentException");
        } catch (const ArgumentException& e) {
            std::cout << "✓ Invalid value correctly rejected: " << e.what() << std::endl;
        }
        
        // Test config file
        std::ofstream configFile("tests/proxy_buffering.conf");
        configFile << "http {\n";
        configFile << "    proxy_request_buffering off;\n";
        configFile << "    \n";
        configFile << "    server {\n";
        configFile << "        listen 80;\n";
        configFile << "        proxy_request_buffering on;\n";
        configFile << "        \n";
        configFile << "        location /api/ {\n";
        configFile << "            proxy_request_buffering off;\n";
        configFile << "        }\n";
        configFile << "    }\n";
        configFile << "}\n";
        configFile.close();
        
        Parser parser = Parser::openConfigFile("tests/proxy_buffering.conf");
        while (parser.getNextDirectives()) {
            IDirective::parseDirective(global, parser.copyDirective());
        }
        
        const HttpDirective* http = global->getFirstChild<HttpDirective>();
        const ProxyRequestBufferingDirective* httpBuff = http->getFirstChild<ProxyRequestBufferingDirective>();
        assert(httpBuff->enabled == false);
        std::cout << "✓ HTTP-level buffering: " << (httpBuff->enabled ? "on" : "off") << std::endl;
        
        std::vector<ServerDirective*> servers = http->getChildren<ServerDirective>();
        const ProxyRequestBufferingDirective* serverBuff = servers[0]->getFirstChild<ProxyRequestBufferingDirective>();
        assert(serverBuff->enabled == true);
        std::cout << "✓ Server-level buffering: " << (serverBuff->enabled ? "on" : "off") << std::endl;
        
        std::vector<LocationDirective*> locations = servers[0]->getChildren<LocationDirective>();
        const ProxyRequestBufferingDirective* locBuff = locations[0]->getFirstChild<ProxyRequestBufferingDirective>();
        assert(locBuff->enabled == false);
        std::cout << "✓ Location-level buffering: " << (locBuff->enabled ? "on" : "off") << std::endl;
        
        std::cout << "\n==================================" << std::endl;
        std::cout << "All tests passed successfully!" << std::endl;
        std::cout << "==================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
