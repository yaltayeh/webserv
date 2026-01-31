#pragma once

#include <string>
#include <map>

namespace libhttp {

	// HTTP Methods
	enum Method {
		GET,
		HEAD,
		POST,
		PUT,
		DELETE,
		INVALID
	};

	// Status Codes
    enum Status {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
		MOVED_PERMANENTLY = 301,
		FOUND = 302,
		NOT_MODIFIED = 304,
        BAD_REQUEST = 400,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
		NOT_ALLOWED = 405,
		REQUEST_TIMEOUT = 408,
		LENGTH_REQUIRED = 411,
		PAYLOAD_TOO_LARGE = 413,
		URI_TOO_LONG = 414,
		REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
		HTTP_VERSION_NOT_SUPPORTED = 505
    };

	// Other / Keys
    const std::string HTTP_1_0 = "HTTP/1.0";
	const std::string HTTP_1_1 = "HTTP/1.1";
	const std::string ContentType = "Content-Type";
	const std::string ContentLength = "Content-Length";
	const std::string Date = "Date";
	const std::string Server = "Server";

	// content types
	extern const std::map<std::string, std::string> MIME_TYPES;

}
