# include <iostream>
# include "Response.hpp"

int main() {
	libhttp::Response res;

	res.setStatus(404);
	res.setHeader("Content-Type", "text/plain");
	res.setHeader("Connection", "close");
	res.setBody("Not Found");

	std::cout << "Initial Response:" << std::endl;
	std::cout << res << std::endl;

	std::cout << "Hello, World!" << std::endl;

	return 0;
}