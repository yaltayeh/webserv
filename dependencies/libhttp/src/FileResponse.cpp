#include "FileResponse.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

namespace libhttp {

	FileResponse::FileResponse(std::string filePath, const std::string &contentType, int statusCode, Method method)
	{
		_method = method;
		
		// Serve regular file using FileResponse for efficiency
		struct stat buf;
		if (stat(filePath.c_str(), &buf) < 0)
			throw std::runtime_error("Failed to stat file: " + filePath);
		
		off_t file_size = buf.st_size;

		std::ostringstream ss;
		ss << HTTP_1_0 << " " << statusCode << " " << getStatusMessage(statusCode) << "\r\n" // Status line
		   << "Content-Type: " << contentType << "\r\n" /// Content-Type header
		   << "Content-Length: " << file_size << "\r\n" // Content-Length header
		   << "\r\n"; // End of headers
		_headerData = ss.str();
		_offset = 0;
		_readBytes = 0;
		_filePath = filePath;
		_fileSize = 0;
	
		if (_method == HEAD) {
			// For HEAD requests, no need to open the file
			_fd = -1;
			return;
		}
		
		int fd = open(filePath.c_str(), O_RDONLY);
		if (fd < 0)
		throw std::runtime_error("Failed to open file: " + filePath);
		
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags < 0)
		{
			close(fd);
			throw std::runtime_error("Failed to get file flags: " + std::string(strerror(errno)));
		}
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			close(fd);
			throw std::runtime_error("Failed to set non-blocking mode on file fd: " + std::string(strerror(errno)));
		}
		
		_fileSize = file_size;
		_fd = fd;
	}

	FileResponse::~FileResponse()
	{
		if (_fd != -1) {
			close(_fd);
		}
	}

	size_t FileResponse::genChunk(char *buffer, size_t max_size)
	{
		size_t header_size = _headerData.size();
		size_t to_copy = 0;
		// Send header data first
		if (_offset < header_size) {
			to_copy = std::min(max_size, header_size - _offset);
			std::memcpy(buffer, _headerData.data() + _offset, to_copy);
			_offset += to_copy;
			max_size -= to_copy;
			if (max_size == 0 || _offset < header_size)
				return to_copy; // Only header data sent
		}
		
		// For HEAD requests, don't send the body - just headers
		// if (_method == HEAD) {
		// 	_readBytes = _fileSize; // Mark as complete
		// 	return to_copy;
		// }
		if (_fd == -1) {
			return to_copy; // File not open
		}
		
		// Once headers are fully sent, start sending body from file
		ssize_t bytes_read = read(_fd, buffer + to_copy, max_size);
		if (bytes_read < 0) {
			std::cerr << "Error reading file: " << _filePath << std::endl;
			return 0; // Error reading file
		}
		_readBytes += bytes_read;
		return bytes_read + to_copy;
	}

	bool FileResponse::hasChunk() const
	{
		return !isComplete();
	}

	bool FileResponse::isComplete() const
	{
		return _offset >= _headerData.size() && _readBytes >= _fileSize;
	}

	std::string FileResponse::toString() const
	{
		return _headerData;
	}
}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::FileResponse &response)
{
	os << response.toString();
	return os;
}