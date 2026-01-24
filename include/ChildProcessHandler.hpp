#pragma once

class ChildProcessHandler;

#include <sys/types.h>
#include "EventLoop.hpp"

/**
 * Abstract base class for handling child processes
 * Provides interface for exit handling and cleanup
 */
class ChildProcessHandler
{
protected:
	pid_t	_pid;

public:
	ChildProcessHandler(pid_t pid);

	virtual ~ChildProcessHandler() = 0;

	pid_t	getPid() const;

	int killChild(int signal);

	virtual void handleChildExit(int status) = 0;
	virtual void handleDestroy(EventLoop *el) = 0;

};
