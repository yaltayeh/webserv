#ifndef UTILS_HPP
#define UTILS_HPP

#include "../include/Constants.hpp"
#include <string>

namespace libhttp {

	static std::string getMimeType(const std::string& path);
	static std::string getStatusMessage(int statusCode);
	static Method stringToMethod(const std::string& methodStr);
}

#endif