#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"
#include "Constants.hpp"

namespace libhttp {

    class DefaultResponse: public Response {
    public:
        DefaultResponse();
        DefaultResponse(int statusCode);
        ~DefaultResponse();

		// Setters
        void setStatus(int statusCode);
        void setHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& body);

		// Getters
		int getStatus() const;
		std::multimap<std::string, std::string> getHeaders() const;
		std::string getBody() const;

		std::string toString() const;

		// Helper methods for generating responses
		void setDefaultPage(int status_code);
		void setAutoindexPage(const std::string& directory_path, const std::string& uri);

    private:
        int _statusCode;
        std::multimap<std::string, std::string> _headers;
        std::string _body;
    };

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::DefaultResponse &response);
