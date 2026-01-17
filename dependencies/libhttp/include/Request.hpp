#pragma once

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
		void setBodyTempFile(const std::string& tempFilePath);

		// Getters
		Method getMethod() const;
		const std::string &getUri() const;
		std::string getHeader(const std::string& key) const;
		const std::string &getBody() const;
		std::multimap<std::string, std::string> getHeaders() const;
		bool hasBodyTempFile() const;
		std::string getBodyTempFile() const;

		// Helper to convert request to string
		std::string toString() const;

	private:
		Method _method;
		std::string _uri;
		std::multimap<std::string, std::string> _headers;
		std::string _body;
		std::string _bodyTempFile; // Path to temp file for large bodies
		bool _hasBodyTempFile;
	};

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Request &request);
