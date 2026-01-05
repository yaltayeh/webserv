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

		// Setters
		void setMethod(Method method);
		void setUri(const std::string& uri);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);

		// Getters
		Method getMethod() const;
		std::string getUri() const;
		std::string getHeader(const std::string& key) const;
		std::string getBody() const;
		std::multimap<std::string, std::string> getHeaders() const;

	private:
		Method _method;
		std::string _uri;
		std::multimap<std::string, std::string> _headers;
		std::string _body;
	};

}

#endif