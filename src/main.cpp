# include <iostream>
# include "Response.hpp"
# include "Parser.hpp"

int main() {
	libhttp::Response res;

	libhttp::Parser parser;
	const std::string http_request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
	int parse_result = parser.parse(http_request.c_str(), http_request.size());

	std::cout << "Parse result: " << parse_result << std::endl;
	// res.setStatus(404);
	// res.setHeader("Content-Type", "text/plain");
	// res.setHeader("Connection", "close");
	// res.setBody("Not Found");

	// std::cout << "Initial Response:" << std::endl;
	// std::cout << res << std::endl;

	// std::cout << "Hello, World!" << std::endl;

	return 0;
}