#pragma once

#include "../include/Constants.hpp"
#include <string>
#include <vector>

namespace libhttp {

	std::string getMimeType(const std::string& path);
	std::string getStatusMessage(int statusCode);
	Method stringToMethod(const std::string& methodStr);
	std::string methodToString(Method method);
	std::vector<std::string> split(const std::string& str, char delimiter);
	std::string trim(const std::string& str);

}
