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
    Directive::g_log_level = Directive::LOG_LEVEL_DEBUG;
    
    LOGI("Starting configuration parser test");
    
    try {
        LOGD("Opening configuration file");
        Directive::Parser parser = Directive::Parser::openConfigFile("tests/complete.conf");
        
        LOGD("Parsing directives...");
        int directive_count = 0;
        while (parser.getNextDirectives()) {
            std::string directive = parser.copyDirective();
            LOGD("Found directive: " << directive);
            Directive::IDirective::parseDirective(Directive::global, directive);
            directive_count++;
        }
        
        LOGI("Parsed " << directive_count << " directives successfully");
        
        // Access configuration
        const Directive::HttpDirective* http = 
            Directive::global->getFirstChild<Directive::HttpDirective>();
        
        if (http) {
            std::vector<Directive::ServerDirective*> servers = 
                http->getChildren<Directive::ServerDirective>();
            LOGI("Found " << servers.size() << " server blocks");
        }
        
        LOGI("Configuration loaded successfully");
        
    } catch (const Directive::FileException& e) {
        LOGE("File error: " << e.what());
        return 1;
    } catch (const Directive::SyntaxException& e) {
        LOGE("Syntax error: " << e.what());
        return 1;
    } catch (const Directive::ArgumentException& e) {
        LOGE("Argument error: " << e.what());
        return 1;
    } catch (const Directive::ConfigException& e) {
        LOGE("Configuration error: " << e.what());
        return 1;
    }
    
    return 0;
}
