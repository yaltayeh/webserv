#include "DefaultResponse.hpp"
#include "ResponseBlender.hpp"
#include <cmath>

namespace libhttp
{

	DefaultResponse::DefaultResponse(const std::string &data)
		: data(data),
		  sent_bytes(0)
	{
	}

	DefaultResponse::~DefaultResponse()
	{
	}

	size_t DefaultResponse::genChunk(char *buffer, size_t max_size)
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

	bool DefaultResponse::hasChunk() const
	{
		return sent_bytes < data.size();
	}

	bool DefaultResponse::isComplete() const
	{
		return sent_bytes >= data.size();
	}

}