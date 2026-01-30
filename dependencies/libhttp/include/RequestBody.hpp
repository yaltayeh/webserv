#pragma once

#include <string>

namespace libhttp
{
	class RequestBody
	{
	private:
		std::string inline_body;	// For small bodies
		std::string temp_file_path; // For large bodies stored in temp file
		bool is_temp_file;
		size_t _contentLength;
		size_t max_inline_size;
		std::string tempDirectory;
		int _fd;

		bool isReadOnly;

		bool isCompletedRead;

		int CreateTempFile();

	public:
		RequestBody();
		RequestBody(size_t max_body_size, const std::string &tempPath);
		RequestBody(const RequestBody &other);
		RequestBody &operator=(const RequestBody &other);
		~RequestBody();

		// Getters
		size_t getContentLength() const;
		bool isTempFile() const;

		bool isCompleted() const;

		ssize_t storeData(const char *data, size_t size);
		ssize_t getChunk(char *buffer, size_t size);
	};
}