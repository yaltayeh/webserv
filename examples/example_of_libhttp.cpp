# include <iostream>
# include "Response.hpp"
# include "Parser.hpp"

int main() {
	libhttp::Parser parser;
	
	const std::string batch_1 = "GET /index.html HTTP/1.0\r\nHost: example.com\r\nConnect";
	const std::string batch_2 = "ion: close\r\nContent-Length: 34\r\n\r\nBody content here";
	const std::string batch_3 = "More body content";
	
	int parse_result = parser.parse(batch_1.c_str(), batch_1.size());
	std::cout << "Parse result: " << parse_result << std::endl;
	
	
	int parse_result_2 = parser.parse(batch_2.c_str(), batch_2.size());
	std::cout << "Parse result 2: " << parse_result_2 << std::endl;
	
	int parse_result_3 = parser.parse(batch_3.c_str(), batch_3.size());
	std::cout << "Parse result 3: " << parse_result_3 << std::endl;

	std::cout << parser.getRequest();
	return 0;
}