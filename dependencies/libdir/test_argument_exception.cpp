// Test ArgumentException functionality
#include "include/Exceptions.hpp"
#include "include/Parser.hpp"
#include "include/IDirective.hpp"
#include "include/Directives/GlobalDirective.hpp"
#include "include/Directives/ExampleDirective.hpp"
#include <iostream>
#include <typeinfo>

using namespace Config;

int main()
{
    // Force linker to include ExampleDirective by referencing its type
    (void)typeid(ExampleDirective);
    
    // Test 1: ArgumentException basic functionality
    std::cout << "Test 1: ArgumentException basic functionality" << std::endl;
    try {
        throw ArgumentException("test_directive", "invalid argument value");
    } catch (const ArgumentException& e) {
        std::cout << "  Caught ArgumentException" << std::endl;
        std::cout << "  Directive: " << e.getDirectiveName() << std::endl;
        std::cout << "  Message: " << e.what() << std::endl;
        std::cout << "  ✓ PASS" << std::endl;
    }
    
    // Test 2: ArgumentException caught as DirectiveException
    std::cout << "\nTest 2: ArgumentException caught as DirectiveException" << std::endl;
    try {
        throw ArgumentException("test_directive", "test message");
    } catch (const DirectiveException& e) {
        std::cout << "  Caught as DirectiveException" << std::endl;
        std::cout << "  Message: " << e.what() << std::endl;
        std::cout << "  ✓ PASS" << std::endl;
    }
    
    // Test 3: ArgumentException caught as ConfigException
    std::cout << "\nTest 3: ArgumentException caught as ConfigException" << std::endl;
    try {
        throw ArgumentException("test_directive", "test message");
    } catch (const ConfigException& e) {
        std::cout << "  Caught as ConfigException" << std::endl;
        std::cout << "  Message: " << e.what() << std::endl;
        std::cout << "  ✓ PASS" << std::endl;
    }
    
    // Test 4: Invalid ExampleDirective usage (too few arguments)
    std::cout << "\nTest 4: ExampleDirective with invalid arguments" << std::endl;
    try {
        // Parse a directive with invalid arguments
        GlobalDirective root;
        const char* configStr = "example;";  // Missing required arguments
        
        IDirective::parseDirective(&root, configStr);
        std::cout << "  ✗ FAIL: Should have thrown ArgumentException" << std::endl;
    } catch (const ArgumentException& e) {
        std::cout << "  Caught ArgumentException from ExampleDirective" << std::endl;
        std::cout << "  Directive: " << e.getDirectiveName() << std::endl;
        std::cout << "  Message: " << e.what() << std::endl;
        std::cout << "  ✓ PASS" << std::endl;
    } catch (const ConfigException& e) {
        std::cout << "  Caught different exception: " << e.what() << std::endl;
    }
    
    // Test 5: Valid ExampleDirective usage
    std::cout << "\nTest 5: ExampleDirective with valid arguments" << std::endl;
    try {
        GlobalDirective root;
        const char* configStr = "example value1 value2;";
        
        IDirective::parseDirective(&root, configStr);
        std::cout << "  ✓ PASS: Directive parsed successfully" << std::endl;
    } catch (const ConfigException& e) {
        std::cout << "  ✗ FAIL: " << e.what() << std::endl;
    }
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}
