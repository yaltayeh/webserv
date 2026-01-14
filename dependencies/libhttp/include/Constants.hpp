#pragma once

#include <string>
#include <map>

namespace libhttp {

	// HTTP Methods
	enum Method {
		GET,
		POST,
		DELETE,
		INVALID
	};

	// Status Codes
    enum Status {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501
    };

	// Other / Keys
    const std::string HTTP_1_0 = "HTTP/1.0";
	const std::string ContentType = "Content-Type";
	const std::string ContentLength = "Content-Length";
	const std::string Date = "Date";
	const std::string Server = "Server";

	// content types
	extern const std::map<std::string, std::string> MIME_TYPES;

}
