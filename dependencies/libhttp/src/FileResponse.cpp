#include "FileResponse.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

namespace libhttp {

	FileResponse::FileResponse(std::string filePath, const std::string &contentType)
	{
		// Serve regular file using FileResponse for efficiency
		struct stat buf;
		if (stat(filePath.c_str(), &buf) < 0)
			throw std::runtime_error("Failed to stat file: " + filePath);
		
		int fd = open(filePath.c_str(), O_RDONLY);
		if (fd < 0)
			throw std::runtime_error("Failed to open file: " + filePath);

		std::cout << "Opened file: " << filePath << " (fd=" << fd << ")\n" << std::endl;
		off_t file_size = buf.st_size;

		std::ostringstream ss;
		ss << HTTP_1_0 << " 200 " << getStatusMessage(200) << "\r\n" // Status line
		   << "Content-Type: " << contentType << "\r\n" /// Content-Type header
		   << "Content-Length: " << file_size << "\r\n" // Content-Length header
		   << "\r\n"; // End of headers
		_headerData = ss.str();
		_offset = 0;
		_readBytes = 0;
		_fileSize = file_size;
		_filePath = filePath;
		_fd = fd;
	}

	FileResponse::~FileResponse()
	{
		close(_fd);
	}

	size_t FileResponse::genChunk(char *buffer, size_t max_size)
	{
		size_t header_size = _headerData.size();
		size_t to_copy = 0;
		// Send header data first
		if (_offset < header_size) {
			to_copy = std::min(max_size, header_size - _offset);
			memcpy(buffer, _headerData.data() + _offset, to_copy);
			_offset += to_copy;
			max_size -= to_copy;
			if (max_size == 0 || _offset < header_size)
				return to_copy; // Only header data sent
		}
		// Once headers are fully sent, start sending body from file
		ssize_t bytes_read = read(_fd, buffer + to_copy, max_size);
		if (bytes_read < 0) {
			perror("Error reading file");
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