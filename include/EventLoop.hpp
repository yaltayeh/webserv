#pragma once

#include <vector>
#include <map>
#include <poll.h>
#include "ListenSocket.hpp"
#include "Config/GlobalConfig.hpp"

class EventLoop
{
private:
    GlobalConfig _config;

    std::vector<VirtualServer *> _virtualServers;
    std::map<uint64_t, ListenSocket *> _listeningSockets;

    int _workerFd;

    int addEvent(int fd, int event, void *udata);
    int deleteEvent(int fd, int event);


public:
    EventLoop(const GlobalConfig &config);
    ~EventLoop();
    EventLoop(const EventLoop &other);
    EventLoop &operator=(const EventLoop &other);

    int init();
    int run();
};
