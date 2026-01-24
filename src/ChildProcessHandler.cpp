#include "ChildProcessHandler.hpp"
#include <signal.h>

ChildProcessHandler::ChildProcessHandler(pid_t pid)
	: _pid(pid)
{
}

ChildProcessHandler::~ChildProcessHandler()
{
}

pid_t ChildProcessHandler::getPid() const
{
	return _pid;
}

int ChildProcessHandler::killChild(int signal)
{
	pid_t pid = _pid;
	if (_pid == -1)
		return -1;
	_pid = -1;
	return kill(pid, signal);
}
