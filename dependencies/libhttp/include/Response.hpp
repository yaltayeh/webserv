#pragma once

#include <cstring>
#include <string>
#include "Request.hpp"

namespace libhttp
{

	class Response
	{
	// protected:
	// 	const Request &associated_request;

	public:
		Response();
		// Response(const Request &associated_request);
		virtual ~Response() = 0;

		virtual size_t genChunk(char *buffer, size_t max_size) = 0;
		virtual bool hasChunk() const = 0;
		virtual bool isComplete() const = 0;
	};

	inline Response::Response() {}
	// inline Response::Response(const Request &associated_request)
	// 	: associated_request(associated_request) {}
	inline libhttp::Response::~Response() {}

}
