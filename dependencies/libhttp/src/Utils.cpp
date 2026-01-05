#include "../include/Constants.hpp"
#include <string>
#include <stdexcept>

namespace libhttp {

	static std::string getMimeType(const std::string& path) {
		size_t dotPos = path.rfind('.');
		if (dotPos != std::string::npos) {
			std::string extension = path.substr(dotPos);
			auto it = MIME_TYPES.find(extension);
			if (it != MIME_TYPES.end()) {
				return it->second;
			}
		}
		return "application/octet-stream"; // Default MIME type
	}
	
	static std::string getStatusMessage(int statusCode) {
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

	static Method stringToMethod(const std::string& methodStr) {
		if (methodStr == "GET") return GET;
		if (methodStr == "POST") return POST;
		if (methodStr == "DELETE") return DELETE;
		throw std::invalid_argument("Invalid HTTP method: " + methodStr);
	}

}