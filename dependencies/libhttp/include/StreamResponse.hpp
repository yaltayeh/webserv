#pragma once

#include "Response.hpp"
#include <sys/types.h>

namespace libhttp
{

	class StreamResponse : public Response
	{
	private:
		int _fd;
		pid_t _pid;

	public:
		StreamResponse();
		StreamResponse(int fd, int pid);
		~StreamResponse();

		int getFd() const;
		int getPid() const;
	};

}
