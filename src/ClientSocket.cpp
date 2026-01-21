#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>
#include "EventLoop.hpp"
#include "StreamResponse.hpp"
#include "DefaultResponse.hpp"
#include "FileResponse.hpp"
#include "StreamSocket.hpp"
#include <unistd.h>

#define READ_BUFFER_SIZE 128





ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
    : Socket(client_fd),
      _addr(addr),
      _listenSocket(listen),
      _file_fd(-1),
      _write_buffer(""),
      _parser(NULL),
      _isResponseSent(false)
{
    // Initialize parser with config from the first virtual server
    const std::vector<VirtualServer *> &servers = _listenSocket->getVirtualServers();
    const ServerConfig &config = servers[0]->getConfig();

    _parser = new libhttp::Parser(config.client_max_body_size, config.client_body_temp_path);
}

ClientSocket::~ClientSocket()
{
    delete _parser;
    if (_file_fd != -1)
        ::close(_file_fd);
    (void)_listenSocket;
}










sockaddr_in ClientSocket::getAddress()
{
    return _addr;
}

const char *ClientSocket::getHost()
{
    char *addr_str = inet_ntoa(_addr.sin_addr);
    return addr_str;
}

void ClientSocket::WriteToBuffer(char *buf, size_t len)
{
    if (_write_buffer.capacity() <= _write_buffer.size() + len)
        _write_buffer.reserve(4096);
    _write_buffer.append(buf, len);
}










int ClientSocket::handleEvent(EventLoop *el)
{
    // --- Sub-case A: Client Disconnected ---
    if (events & (IO_ERROR | IO_HUP))
    {
        std::cout << "client [" << getHost() << "]: disconnected" << std::endl;

        // Clean up: remove from event loop and free memory
        el->deleteEvent(_socketfd, IO_READ, this);
        el->deleteEvent(_socketfd, IO_WRITE, this);
        delete this;
        return 0;
    }

    // --- Sub-case B: Data Ready to Read (HTTP Request) ---
    if (events & IO_READ)
    {
        libhttp::State state = handleRead();
        // std::cout << "Parser state after read: " << state << std::endl;

        if (state == libhttp::CLOSED)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            el->deleteEvent(_socketfd, IO_WRITE, this);
            delete this;
            return (0);
        }

        if (state == libhttp::BODY_EXCEEDED_MAX_SIZE)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            el->addEvent(_socketfd, IO_WRITE, this);

            libhttp::DefaultResponse def = libhttp::DefaultResponse(413);
            _write_buffer = def.toString();
        }

        if (state == libhttp::ERROR)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            el->addEvent(_socketfd, IO_WRITE, this);

            libhttp::DefaultResponse def = libhttp::DefaultResponse(400);
            _write_buffer = def.toString();
        }
        
        if (state == libhttp::COMPLETE)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            el->addEvent(_socketfd, IO_WRITE, this);

            const libhttp::Request request = _parser->getRequest();
            const VirtualServer *owner_server = VirtualServer::findMatchServer(
                _listenSocket->getVirtualServers(),
                request
            );
            const libhttp::Response *raw_response = owner_server->handleRequest(request);
            
            if (dynamic_cast<const libhttp::DefaultResponse *>(raw_response))
            { 
                const libhttp::DefaultResponse *response = dynamic_cast<const libhttp::DefaultResponse *>(raw_response);
                _write_buffer = response->toString();
            }
            
            else if (dynamic_cast<const libhttp::StreamResponse *>(raw_response))
            {
                const libhttp::StreamResponse *response = dynamic_cast<const libhttp::StreamResponse *>(raw_response);
                _write_buffer = "";
                StreamSocket *ss = new StreamSocket(response->getFd(), this);
                el->addEvent(ss->getSocketfd(), IO_READ, ss);
            }
            
            else if (dynamic_cast<const libhttp::FileResponse *>(raw_response))
            {
                const libhttp::FileResponse *response = dynamic_cast<const libhttp::FileResponse *>(raw_response);
                _write_buffer = response->toString();
                _file_fd = response->getFd();
            }
            
            delete raw_response;
        }
    }

    // --- Sub-case C: Socket Ready to Write (HTTP Response) ---
    if (events & IO_WRITE)
    {
        std::cout << "WE ARE READY TO WRITE BOSS " << getHost() << std::endl;

        // Only remove write event if buffer is empty AND no file to send
        if (_write_buffer.empty() && _file_fd == -1)
        {
            el->deleteEvent(_socketfd, IO_WRITE, this);
            return (1);
        }
        // Send HTTP response data
        handleWrite();

        // If entire response has been sent
        if (_isResponseSent)
        {
            // Clean up: close connection and free memory
            el->deleteEvent(_socketfd, IO_WRITE, this);
            el->deleteEvent(_socketfd, IO_READ, this);
            delete this; // defualt close _socketfd
        }
    }
    return (0);
}














libhttp::State ClientSocket::handleRead()
{
    char buffer[READ_BUFFER_SIZE];
    ssize_t bytes_read = recv(buffer, READ_BUFFER_SIZE);

    if (bytes_read <= 0)
    {
        std::cout << "Client " << getHost() << " closed connection" << std::endl;
        return libhttp::CLOSED;
    }

    std::cout << "Read " << bytes_read << " bytes from client " << getHost() << std::endl;
    return _parser->parse(buffer, bytes_read);
}

bool ClientSocket::handleWrite()
{
    // If buffer is empty and we have a file to send, read next chunk
    if (_write_buffer.empty() && _file_fd != -1)
    {
        char file_buffer[8192];
        ssize_t bytes_read = read(_file_fd, file_buffer, sizeof(file_buffer));
        
        if (bytes_read < 0)
        {
            std::cerr << "Error reading from file for client " << getHost() << std::endl;
            ::close(_file_fd);
            _file_fd = -1;
            return false;
        }
        
        if (bytes_read == 0)
        {
            // End of file reached
            std::cout << "File completely sent to client " << getHost() << std::endl;
            ::close(_file_fd);
            _file_fd = -1;
            _isResponseSent = 1;
            return true;
        }
        
        _write_buffer.append(file_buffer, bytes_read);
        std::cout << "Read " << bytes_read << " bytes from file for client " << getHost() << std::endl;
    }
    
    if (_write_buffer.empty())
        return true;

    ssize_t bytes_sent = send(_write_buffer.c_str(), _write_buffer.size());

    if (bytes_sent < 0)
    {
        std::cerr << "Error writing to client " << getHost() << std::endl;
        return false;
    }

    if (bytes_sent == 0)
        return true;

    std::cout << "Sent " << bytes_sent << " bytes to client " << getHost() << std::endl;

    _write_buffer.erase(0, bytes_sent);

    // Check if all data sent
    if (_write_buffer.empty())
    {
        // If no more file to send, we're done
        if (_file_fd == -1)
        {
            std::cout << "All data sent to client " << getHost() << std::endl;
            _isResponseSent = 1;
        }
        // Otherwise, we'll read more from the file on next write event
    }
    return true;
}
