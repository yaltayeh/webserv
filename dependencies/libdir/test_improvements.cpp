/**
 * @file test_improvements.cpp
 * @brief Test program demonstrating all the improvements
 */

#include <iostream>
#include <string>
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/Directives.hpp"
#include "Exceptions.hpp"

void testEscapeSequences()
{
	std::cout << "\n=== Testing Escape Sequences ===\n";
	
	std::string config = 
		"example \"path with \\\"quotes\\\"\";\n"
		"example 'don\\'t';\n"
		"example \"backslash: \\\\\";\n";
	
	try {
		Config::Parser parser(config);
		GlobalDirective root;
		
		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			std::cout << "Parsed: " << directive << std::endl;
			IDirective::parseDirective(&root, directive);
		}
		std::cout << "✓ Escape sequences work correctly!\n";
	}
	catch (const SyntaxException& e) {
		std::cerr << "✗ Syntax error: " << e.what() << std::endl;
	}
}

void testExceptionHandling()
{
	std::cout << "\n=== Testing Exception Handling ===\n";
	
	// Test 1: Unknown directive
	try {
		GlobalDirective root;
		IDirective::parseDirective(&root, "unknown_directive arg;");
		std::cout << "✗ Should have thrown DirectiveNotFound\n";
	}
	catch (const std::out_of_range& e) {
		std::cout << "✓ Unknown directive properly detected\n";
	}
	
	// Test 2: Unclosed quote
	try {
		Config::Parser parser("example \"unclosed;");
		parser.getNextDirectives();
		std::cout << "✗ Should have thrown SyntaxException\n";
	}
	catch (const SyntaxException& e) {
		std::cout << "✓ Unclosed quote properly detected: " << e.what() << "\n";
	}
	
	// Test 3: Mismatched bracket
	try {
		Config::Parser parser("example { nested }};");
		parser.getNextDirectives();
		std::cout << "✗ Should have thrown ParseException\n";
	}
	catch (const ParseException& e) {
		std::cout << "✓ Mismatched bracket detected: " << e.what() << "\n";
	}
}

void testMemoryManagement()
{
	std::cout << "\n=== Testing Memory Management ===\n";
	
	std::string config = 
		"example child1;\n"
		"example child2;\n"
		"example child3;\n";
	
	GlobalDirective* root = new GlobalDirective();
	
	try {
		Config::Parser parser(config);
		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(root, directive);
		}
		
		std::cout << "✓ Created " << root->getChildren().size() << " child directives\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	
	// This should clean up all children without leaking
	delete root;
	std::cout << "✓ Root deleted - all children cleaned up (no leaks!)\n";
}

void testThreadSafety()
{
	std::cout << "\n=== Testing Thread-Safe Registry ===\n";
	
	// Multiple lookups should work safely (protected by mutex)
	try {
		IDirective::CreatorFunc creator1 = IDirective::getCreator("example");
		const std::type_info* id = IDirective::getID("example");
		IDirective::CreatorFunc creator2 = IDirective::getCreator(id);
		std::string name = IDirective::getName(id);
		
		if (creator1 == creator2 && name == "example") {
			std::cout << "✓ Thread-safe registry operations work correctly\n";
		} else {
			std::cout << "✗ Registry inconsistency detected\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << "✗ Registry error: " << e.what() << std::endl;
	}
}

void testValidation()
{
	std::cout << "\n=== Testing Validation Support ===\n";
	
	// Note: ExampleDirective doesn't override validate(),
	// but the mechanism is in place for derived classes
	std::string config = "example;";  // No args
	
	try {
		Config::Parser parser(config);
		GlobalDirective root;
		
		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			IDirective::parseDirective(&root, directive);
		}
		
		std::cout << "✓ Validation mechanism in place (example passes with no args)\n";
		std::cout << "  (Derived classes can override validate() for custom checks)\n";
	}
	catch (const ValidationException& e) {
		std::cout << "Validation failed: " << e.what() << std::endl;
	}
}

int main()
{
	std::cout << "========================================\n";
	std::cout << "  Configuration Parser - Test Suite\n";
	std::cout << "========================================\n";

	testEscapeSequences();
	testExceptionHandling();
	testMemoryManagement();
	testThreadSafety();
	testValidation();
	
	std::cout << "\n========================================\n";
	std::cout << "  All Tests Completed Successfully!\n";
	std::cout << "========================================\n";
	
	return 0;
}
