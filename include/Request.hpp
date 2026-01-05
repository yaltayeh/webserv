#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include "Constants.hpp"

namespace libhttp {

	class Request {
	public:
		Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request();
	private:
		Method _method;
		std::string _uri;
		std::map<std::string, std::string> _headers;
		std::string _body;
	};

}

#endif