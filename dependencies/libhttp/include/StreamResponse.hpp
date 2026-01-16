#pragma once

#include "Response.hpp"

namespace libhttp
{

	class StreamResponse: public Response
	{
	private:
		int _fd;

	public:
		StreamResponse();
		StreamResponse(int fd);
		~StreamResponse();

		int getFd() const;
		void setFd(int fd);
	};

}
