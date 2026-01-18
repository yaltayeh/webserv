#include "../include/Utils.hpp"
#include "../include/Constants.hpp"
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace libhttp {
	
	std::string getStatusMessage(int statusCode) {
		switch (statusCode) {
			case OK: return "OK";
			case CREATED: return "Created";
			case NO_CONTENT: return "No Content";
			case NOT_ALLOWED: return "Method Not Allowed";
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

	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> v;
		std::string item;
		std::stringstream ss(str);
		while (std::getline(ss, item, delimiter))
			v.push_back(item);
		return v;
	}

	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t\n\r");
		if (first == std::string::npos)
			return "";
		size_t last = str.find_last_not_of(" \t\n\r");
		return str.substr(first, (last - first + 1));
	}

}
