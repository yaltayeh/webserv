#include <iostream>
#include <cerrno>
#include <cstring>
#include "EventLoop.hpp"

#if defined(__linux__)
#define EVENT_READ EPOLLIN
#define EVENT_WRITE EPOLLOUT
#define EVENT_EDGE EPOLLET
#elif defined(__APPLE__) && defined(__MACH__)
#include <sys/event.h>
#define EVENT_READ EVFILT_READ
#define EVENT_WRITE EVFILT_WRITE
#endif

EventLoop::EventLoop(const GlobalConfig &config)
    : _config(config), _workerFd(-1) {}

EventLoop::~EventLoop()
{
    for (std::map<uint64_t, ListenSocket *>::iterator it = _listeningSockets.begin();
         it != _listeningSockets.end();
         ++it)
    {
        (*it).second->close();
        delete (*it).second;
    }
    for (std::vector<VirtualServer *>::iterator it = _virtualServers.begin();
         it != _virtualServers.end();
         ++it)
    {
        delete (*it);
    }
    _virtualServers.clear();
    _listeningSockets.clear();
}

EventLoop::EventLoop(const EventLoop &other)
{
    this->_config = other._config;
}

EventLoop &EventLoop::operator=(const EventLoop &other)
{
    if (this != &other)
    {
        this->_config = other._config;
    }
    return (*this);
}

int EventLoop::init()
{
    ListenSocket *socket = NULL;
    VirtualServer *virtualServer = NULL;

    for (size_t i = 0; i < _config.https.size(); i++)
    {
        const std::vector<ServerConfig> &servers = _config.https[i].servers;
        for (size_t j = 0; j < servers.size(); j++)
        {
            virtualServer = new VirtualServer(servers[j]);
            _virtualServers.push_back(virtualServer);

            const std::vector<ServerConfig::ListenAddr> &addrs = servers[j].listenAddresses;
            for (size_t l = 0; l < addrs.size(); l++)
            {
                ServerConfig::ListenAddr addr = addrs[l];
                if (_listeningSockets.find(addr) == _listeningSockets.end())
                {
                    socket = new ListenSocket(addr.ip, addr.port);
                    if (socket->getSocketfd() < 0)
                    {
                        delete socket;
                        return (-1);
                    }
                    _listeningSockets[(uint64_t)addr] = socket;
                }
                else
                {
                    socket = _listeningSockets[(uint64_t)addr];
                }
                socket->addVirtualServer(virtualServer);
            }
        }
    }
    return (0);
}

int EventLoop::addEvent(int fd, int event, void *udata)
{
    if (_workerFd < 0)
        return (-1);

#if defined(__linux__)
    struct epoll_event ev;
    ev.events = event | EPOLLET;
    ev.data.ptr = udata;
    if (epoll_ctl(_workerFd, EPOLL_CTL_ADD, fd, &ev) == -1)
        perror("epoll_ctl ADD");

#elif defined(__APPLE__) && defined(__MACH__)
    struct kevent kev;
    EV_SET(&kev, fd, event,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           0, 0, udata);
    if (kevent(_workerFd, &kev, 1, NULL, 0, NULL) == -1)
        perror("kevent ADD");
#endif

    return 0;
}

int EventLoop::deleteEvent(int fd, int event)
{
    if (_workerFd < 0)
        return (-1);

#if defined(__linux__)
    if (epoll_ctl(_workerFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        perror("epoll_ctl DELL");
        
#elif defined(__APPLE__) && defined(__MACH__)
    struct kevent kev;
    EV_SET(&kev, fd,
           event,
           EV_DELETE, 0, 0,
           NULL);
    if (kevent(_workerFd, &kev, 1, NULL, 0, NULL) == -1)
        perror("kevent DELL");
#endif

    return 0;
}

int EventLoop::run()
{
    _workerFd = kqueue();
    if (_workerFd < 0)
    {
        perror("kqueue");
        return (-1);
    }

    // add listen sockets to kqueue
    for (std::map<uint64_t, ListenSocket *>::iterator it = _listeningSockets.begin();
         it != _listeningSockets.end();
         ++it)
    {
        addEvent((*it).second->getSocketfd(),
                 EVENT_READ,
                 (*it).second);
    }

    struct kevent events[32];

    while (1)
    {
        int n = kevent(_workerFd, NULL, 0, events, sizeof(events) / sizeof(events[0]), NULL);
        for (int i = 0; i < n; i++)
        {
            Socket *socket = (Socket *)events[i].udata;
            if (socket && socket->getType() == LISTEN_ID)
            {
                ListenSocket *listen = dynamic_cast<ListenSocket *>(socket);
                while (1)
                {
                    ClientSocket *clientSocket = listen->accept();
                    if (!clientSocket)
                    {
                        if (errno == EAGAIN)
                            break;
                        perror("accept");
                        return -1;
                        break;
                    }
                    addEvent(clientSocket->getSocketfd(),
                             EVFILT_READ,
                             clientSocket);
                    std::cout << "add client " << clientSocket->getHost() << std::endl;
                }
                continue;
            }
            else if (socket && socket->getType() == CLIENT_ID)
            {
                ClientSocket *client = dynamic_cast<ClientSocket *>(socket);
                /* Client disconnected */
                if (events[i].flags & EV_EOF)
                {
                    std::cout << "client [" << client->getHost() << "]: disconnected" << std::endl;
                    deleteEvent(client->getSocketfd(), EVFILT_READ);
                    delete client;
                    continue;
                }
                if (events[i].filter == EVFILT_READ)
                {
                    if (client->handleRead() == false)
                    {
                        deleteEvent(client->getSocketfd(), EVFILT_READ);
                        delete client;
                    }
                    if (client->isComplateResponse())
                    {
                        deleteEvent(client->getSocketfd(), EVFILT_READ);
                        addEvent(client->getSocketfd(), EVFILT_WRITE, client);
                    }
                }
                else if (events[i].filter == EVFILT_WRITE)
                {
                    // std::cout << "---- EVFILT_WRITE ----" << std::endl;
                    client->handleWrite();
                    if (client->isSendResponse())
                    {
                        deleteEvent(client->getSocketfd(), EVFILT_WRITE);
                        delete client;
                    }
                }
            }
        }
    }
    return (0);
}
