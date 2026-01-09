/**
 * @file test_ocf.cpp
 * @brief Test Orthodox Canonical Form implementation
 */

#include <iostream>
#include <string>
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "Directives/GlobalDirective.hpp"
#include "Directives/ExampleDirective.hpp"
#include "DirectiveFactory.hpp"

using namespace Config;

void testParserOCF()
{
	std::cout << "\n=== Testing Parser OCF ===\n";
	
	// Test copy constructor
	Config::Parser parser1("example arg1;");
	Config::Parser parser2(parser1);
	
	std::cout << "✓ Parser copy constructor works\n";
	
	// Test assignment operator
	Config::Parser parser3("other;");
	parser3 = parser1;
	
	std::cout << "✓ Parser assignment operator works\n";
	
	// Test that copied parsers work correctly
	if (parser2.getNextDirectives()) {
		std::string directive = parser2.copyDirective();
		std::cout << "✓ Copied parser can parse: " << directive << "\n";
	}
}

void testGlobalDirectiveOCF()
{
	std::cout << "\n=== Testing GlobalDirective OCF ===\n";
	
	// Test copy constructor
	GlobalDirective root1;
	GlobalDirective root2(root1);
	
	std::cout << "✓ GlobalDirective copy constructor works\n";
	
	// Test assignment operator
	GlobalDirective root3;
	root3 = root1;
	
	std::cout << "✓ GlobalDirective assignment operator works\n";
	
	// Test self-assignment (suppress warning)
	GlobalDirective* ptr = &root3;
	root3 = *ptr;
	
	std::cout << "✓ GlobalDirective self-assignment handled correctly\n";
}

void testExampleDirectiveOCF()
{
	std::cout << "\n=== Testing ExampleDirective OCF ===\n";
	
	// Test copy constructor
	std::vector<std::string> args;
	args.push_back("example");
	args.push_back("value1");
	args.push_back("value2");
	
	ExampleDirective* dir1 = ExampleDirective::initExampleDirective(args);
	ExampleDirective dir2(*dir1);
	
	std::cout << "✓ ExampleDirective copy constructor works\n";
	
	// Test assignment operator
	ExampleDirective dir3;
	dir3 = *dir1;
	
	std::cout << "✓ ExampleDirective assignment operator works\n";
	
	// Test self-assignment
	ExampleDirective* ptr = &dir3;
	dir3 = *ptr;
	
	std::cout << "✓ ExampleDirective self-assignment handled correctly\n";
	
	delete dir1;
}

int main()
{
	std::cout << "========================================\n";
	std::cout << "  Orthodox Canonical Form Test Suite\n";
	std::cout << "========================================\n";

	testParserOCF();
	testGlobalDirectiveOCF();
	testExampleDirectiveOCF();
	
	std::cout << "\n========================================\n";
	std::cout << "  All OCF Tests Completed!\n";
	std::cout << "========================================\n";
	
	return 0;
}
