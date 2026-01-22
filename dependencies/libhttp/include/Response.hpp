#pragma once

#include <cstring>
#include <string>

namespace libhttp
{

	class Response
	{
	private:
		std::string data;
		size_t sent_bytes;

	public:
		Response();
		virtual ~Response() = 0;


		virtual size_t genChunk(char *buffer, size_t max_size);
		virtual bool isHasChunk() const;
		virtual bool isComplete() const;

	};
}
