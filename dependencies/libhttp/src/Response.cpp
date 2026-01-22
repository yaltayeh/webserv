#include "Response.hpp"
#include <cmath>

libhttp::Response::Response()
	: data(
		  "HTTP/1.1 200 OK\r\n"
		  "Content-Type: text/plain\r\n"
		  "\r\n"
		  "Hello, World!\r\n"),
	  sent_bytes(0)
{
}

libhttp::Response::~Response()
{
}

size_t libhttp::Response::genChunk(char *buffer, size_t max_size)
{
	size_t bytes_left = data.size() - sent_bytes;

	size_t chunk_size = std::min(bytes_left, max_size);

	if (chunk_size > 0)
	{
		std::memcpy(buffer, data.c_str() + sent_bytes, chunk_size);
		sent_bytes += chunk_size;
	}

	return chunk_size;
}

bool libhttp::Response::isHasChunk() const
{
	return sent_bytes < data.size();
}

bool libhttp::Response::isComplete() const
{
	return sent_bytes >= data.size();
}
