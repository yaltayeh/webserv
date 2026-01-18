#pragma once

#include <string>
#include <map>
#include "Response.hpp"
#include "Constants.hpp"

namespace libhttp {

    class StaticResponse: public Response {
    public:
        StaticResponse();
        StaticResponse(const StaticResponse& other);
        StaticResponse& operator=(const StaticResponse& other);
        ~StaticResponse();

		// Setters
        void setStatus(int statusCode);
        void setHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& body);

		// Getters
		int getStatus() const;
		std::multimap<std::string, std::string> getHeaders() const;
		std::string getBody() const;

		// Helper to convert StaticResponse to string
		std::string toString() const;
		std::string toString(bool includeBody) const;		
    private:
        int _statusCode;
        std::multimap<std::string, std::string> _headers;
        std::string _body;
    };

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::StaticResponse &StaticResponse);
