#ifndef UTILS_HPP
#define UTILS_HPP

#include "../include/Constants.hpp"
#include <string>

namespace libhttp {

	std::string getMimeType(const std::string& path);
	std::string getStatusMessage(int statusCode);
	Method stringToMethod(const std::string& methodStr);
}

#endif