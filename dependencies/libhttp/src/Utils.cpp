#include "../include/Utils.hpp"
#include "../include/Constants.hpp"
#include <string>
#include <stdexcept>

namespace libhttp {

	std::string getMimeType(const std::string& path) {
		size_t dotPos = path.rfind('.');
		if (dotPos != std::string::npos) {
			std::string extension = path.substr(dotPos);
			std::map<std::string, std::string>::const_iterator it = MIME_TYPES.find(extension);
			if (it != MIME_TYPES.end()) {
				return it->second;
			}
		}
		return "application/octet-stream"; // Default MIME type
	}
	
	std::string getStatusMessage(int statusCode) {
		switch (statusCode) {
			case OK: return "OK";
			case CREATED: return "Created";
			case NO_CONTENT: return "No Content";
			case BAD_REQUEST: return "Bad Request";
			case FORBIDDEN: return "Forbidden";
			case NOT_FOUND: return "Not Found";
			case INTERNAL_SERVER_ERROR: return "Internal Server Error";
			case NOT_IMPLEMENTED: return "Not Implemented";
			default: return "Unknown Status";
		}
	}

	Method stringToMethod(const std::string& methodStr) {
		if (methodStr == "GET") return GET;
		if (methodStr == "POST") return POST;
		if (methodStr == "DELETE") return DELETE;
		return INVALID; // Default to INVALID for unknown methods
	}

	std::string methodToString(Method method) {
		switch (method) {
			case GET: return "GET";
			case POST: return "POST";
			case DELETE: return "DELETE";
			default: return "INVALID";
		}
	}
}