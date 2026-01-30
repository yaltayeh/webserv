#pragma once

#include <string>
#include <map>
#include "Constants.hpp"
#include "RequestBody.hpp"

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
		void setBody(const RequestBody& body);

		// Getters
		Method getMethod() const;
		const std::string &getUri() const;
		std::string getHeader(const std::string& key) const;
		const RequestBody &getBody() const;
		const std::multimap<std::string, std::string> &getHeaders() const;

		// Helper to convert request to string
		std::string toString() const;

	private:
		Method _method;
		std::string _uri;
		std::multimap<std::string, std::string> _headers;
		RequestBody _body;
	};

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Request &request);
