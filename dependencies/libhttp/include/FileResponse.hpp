#pragma once

#include <string>
#include <map>
#include "Response.hpp"
#include "Constants.hpp"

namespace libhttp {

    class FileResponse: public Response {
    public:
        FileResponse();
        ~FileResponse();

		// Setters
        void setStatus(int statusCode);
        void setHeader(const std::string& key, const std::string& value);
        void setFd(int fd);

		// Getters
		int getStatus() const;
		std::multimap<std::string, std::string> getHeaders() const;
		int getFd() const;

		std::string toString() const;
    private:
        int _statusCode;
        std::multimap<std::string, std::string> _headers;
        int         _fd;
    };

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::FileResponse &response);