#include "FileResponse.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <sstream>

namespace libhttp {

	FileResponse::FileResponse()
		: _statusCode(200), _headers(), _fd(-1) {}

	FileResponse::~FileResponse() {}

	// Setters
	void FileResponse::setStatus(int statusCode) {
		_statusCode = statusCode;
	}
	void FileResponse::setHeader(const std::string& key, const std::string& value) {
		_headers.insert(std::make_pair(key, value));
	}
	void FileResponse::setFd(int fd) {
		_fd = fd;
	}

	// Getters
	int FileResponse::getStatus() const { return _statusCode; }
	std::multimap<std::string, std::string> FileResponse::getHeaders() const { return _headers; }
	int FileResponse::getFd() const { return _fd; }

	std::string FileResponse::toString() const {
		std::string response;
		response += HTTP_1_0;
		std::stringstream ss;
		ss << _statusCode;
		response += " " + ss.str() + " " + getStatusMessage(_statusCode) + "\r\n";
		for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			response += it->first + ": " + it->second + "\r\n";
		}
		response += "\r\n";
		return response;
	}

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::FileResponse &response)
{
	os << response.toString();
	return os;
}