#pragma once

#include <string>
#include <map>
#include "Constants.hpp"

namespace libhttp {

    class Response {
    public:
        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        ~Response();

		// Setters
        void setStatus(int statusCode);
        void setHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& body);

		// Getters
		int getStatus() const;
		std::multimap<std::string, std::string> getHeaders() const;
		std::string getBody() const;

		// Helper to convert response to string
		std::string toString() const;		
    private:
        int _statusCode;
        std::multimap<std::string, std::string> _headers;
        std::string _body;
    };

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::Response &response);
