// Example of using the logging system
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Logger.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include <iostream>

int main() {
    // Set log level for debugging
    Config::g_log_level = Config::LOG_LEVEL_DEBUG;
    
    LOGI("Starting configuration parser test");
    
    try {
        LOGD("Opening configuration file");
        Config::Parser parser = Config::Parser::openConfigFile("tests/complete.conf");
        
        LOGD("Parsing directives...");
        int directive_count = 0;
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            LOGD("Found directive: " << directive);
            Config::IDirective::parseDirective(Config::global, directive);
            directive_count++;
        }
        
        LOGI("Parsed " << directive_count << " directives successfully");
        
        // Access configuration
        const Config::HttpDirective* http = 
            Config::global->getFirstChild<Config::HttpDirective>();
        
        if (http) {
            std::vector<Config::ServerDirective*> servers = 
                http->getChildren<Config::ServerDirective>();
            LOGI("Found " << servers.size() << " server blocks");
        }
        
        LOGI("Configuration loaded successfully");
        
    } catch (const Config::FileException& e) {
        LOGE("File error: " << e.what());
        return 1;
    } catch (const Config::SyntaxException& e) {
        LOGE("Syntax error: " << e.what());
        return 1;
    } catch (const Config::ArgumentException& e) {
        LOGE("Argument error: " << e.what());
        return 1;
    } catch (const Config::ConfigException& e) {
        LOGE("Configuration error: " << e.what());
        return 1;
    }
    
    return 0;
}
