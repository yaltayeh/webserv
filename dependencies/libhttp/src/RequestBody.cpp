#include "RequestBody.hpp"
#include <unistd.h>
#include <fcntl.h>

namespace libhttp
{
	int RequestBody::CreateTempFile()
	{
		if (is_temp_file)
			return 0; // Already a temp file
		
		std::string temp_filename = tempDirectory + "/libhttp_body_XXXXXX";
		_fd = mkostemp(&temp_filename[0], O_RDWR | O_CLOEXEC | O_NONBLOCK);
		if (_fd == -1)
			return -1; // Failed to create temp file
		temp_file_path = std::string(temp_filename);
		is_temp_file = true;

		// Write existing inline data to temp file
		if (!inline_body.empty())
		{
			if (write(_fd, inline_body.c_str(), inline_body.size()) != (ssize_t)inline_body.size())
			{
				close(_fd);
				return -1;
			}
			inline_body.clear();
		}
		return 0;
	}

	RequestBody::RequestBody()
		: is_temp_file(false),
		  _contentLength(0),
		  max_inline_size(1024 * 1024), // Default 1MB inline size
		  tempDirectory("/tmp"),
		  _fd(-1),
		  isReadOnly(false),
		  isCompletedRead(false)
	{
	}

	RequestBody::RequestBody(size_t max_body_size, const std::string &tempPath)
		: is_temp_file(false),
		  _contentLength(0),
		  max_inline_size(max_body_size),
		  tempDirectory(tempPath),
		  _fd(-1),
		  isReadOnly(false),
		  isCompletedRead(false)
	{
	}

	RequestBody::RequestBody(const RequestBody &other)
		: inline_body(other.inline_body),
		  temp_file_path(other.temp_file_path),
		  is_temp_file(other.is_temp_file),
		  _contentLength(other._contentLength),
		  max_inline_size(other.max_inline_size),
		  isReadOnly(true),
		  isCompletedRead(false)
	{
		// Duplicate file descriptor if valid
		if (other._fd != -1)
			_fd = open(other.temp_file_path.c_str(), O_RDONLY | O_NONBLOCK);
		else
			_fd = -1;
	}

	RequestBody &RequestBody::operator=(const RequestBody &other)
	{
		if (this != &other)
		{
			inline_body = other.inline_body;
			temp_file_path = other.temp_file_path;
			is_temp_file = other.is_temp_file;
			_contentLength = other._contentLength;
			max_inline_size = other.max_inline_size;
			isReadOnly = true;
			isCompletedRead = false;
			// Duplicate file descriptor if valid
			if (other._fd != -1)
				_fd = open(other.temp_file_path.c_str(), O_RDONLY | O_NONBLOCK);
			else
				_fd = -1;
		}
		return *this;
	}

	RequestBody::~RequestBody()
	{
		if (_fd != -1)
			close(_fd);
	}

	size_t RequestBody::getContentLength() const
	{
		return _contentLength;
	}

	bool RequestBody::isTempFile() const
	{
		return is_temp_file;
	}

	bool RequestBody::isCompleted() const
	{
		return isCompletedRead;
	}

	ssize_t RequestBody::storeData(const char *data, size_t size)
	{
		if (isReadOnly)
			return -1; // Cannot store data in read-only copy
		// Check if we can store inline
		if (!is_temp_file && (_contentLength + size <= max_inline_size))
		{
			inline_body.append(data, size);
			_contentLength += size;
			return size;
		}
		else
		{
			// Switch to temp file storage if not already
			if (!is_temp_file && CreateTempFile() != 0)
				return -1; // Failed to create temp file

			// Write new data to temp file
			ssize_t bytes_written = write(_fd, data, size);
			if (bytes_written == -1)
				return -1; // Write error
			_contentLength += bytes_written;
			return bytes_written;
		}
	}

	ssize_t RequestBody::getChunk(char *buffer, size_t size)
	{
		if (is_temp_file)
		{
			if (_fd == -1)
				return -1; // Invalid file descriptor
			ssize_t bytes_read = read(_fd, buffer, size);
			if (bytes_read == 0)
				isCompletedRead = true;
			return bytes_read;
		}
		else
		{
			// Read from inline body
			size_t bytes_to_read = std::min(size, inline_body.size());
			memcpy(buffer, inline_body.c_str(), bytes_to_read);
			inline_body.erase(0, bytes_to_read);
			if (inline_body.empty())
				isCompletedRead = true;
			return bytes_to_read;
		}
	}
}