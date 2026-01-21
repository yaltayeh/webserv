#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>
#include "EventLoop.hpp"
#include "StreamResponse.hpp"
#include "DefaultResponse.hpp"
#include "StreamSocket.hpp"

#define READ_BUFFER_SIZE 2

ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
    : Socket(client_fd),
      _addr(addr),
      _listenSocket(listen),
      _parser(NULL),
      _isRequestRead(false),
      _isResponseSent(false)
{
    // Initialize parser with config from the first virtual server
    const std::vector<VirtualServer *> &servers = _listenSocket->getVirtualServer();
    const ServerConfig &config = servers[0]->getConfig();

    _parser = new libhttp::Parser(config.client_max_body_size, config.client_body_temp_path);
}

ClientSocket::~ClientSocket()
{
    delete _parser;
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
        std::cout << "HERE WE HAVE A COMPLETE REQUEST FROM " << getHost() << std::endl;

        libhttp::State state = handleRead();

        std::cout << "Parser state after read: " << state << std::endl;
        
        if (state == libhttp::COMPLETE)
        {
            // el->deleteEvent(_socketfd, IO_WRITE, this);
            // el->deleteEvent(_socketfd, IO_READ, this);
            // delete this;
            // return (0);

            el->deleteEvent(_socketfd, IO_READ, this);
            el->addEvent(_socketfd, IO_WRITE, this);

            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << _parser->getRequest();
            std::cout << std::endl;

            libhttp::DefaultResponse def = libhttp::DefaultResponse();
            def.setDefaultPage(200);
            _write_buffer = def.toString();
        }
        /*
        // Read data and parse HTTP request
        if (handleRead() == false) // Returns false on error
        {
            // TODO: Generate 400 Bad Request response
            
            // Error occurred, close connection
            el->deleteEvent(_socketfd, IO_WRITE, this);
            el->deleteEvent(_socketfd, IO_READ, this);
            delete this;
            return (0);
        }
        */

        /*
        // If HTTP request is complete and response is ready
        if (_isRequestRead)
        {
            // Switch from reading to writing
            // (Stop monitoring reads, start monitoring writes)
            el->deleteEvent(_socketfd, IO_READ, this);
            el->addEvent(_socketfd, IO_WRITE, this);

            // Route the request and generate response
            libhttp::Request request = _parser->getRequest();
            const std::vector<VirtualServer *> &servers = _listenSocket->getVirtualServer();
            if (servers.empty())
            {
                std::cerr << "Error: No virtual servers configured" << std::endl;
                return false;
            }

            
            // TODO: assign virtual server function
            // TODO: turn the request ownership to the selected virtual server

            // const VirtualServer *server = VirtualServer::findMatchServer(servers, request);
            // if (!server)
            // {
            //     std::cerr << "Virtual Server Not Found" << std::endl;
            //     return (0);
            // }
            // const libhttp::Response *raw_response = &server->handleRequest(request);
            // if (dynamic_cast<const libhttp::DefaultResponse *>(raw_response))
            // {
            //     const libhttp::DefaultResponse *response = dynamic_cast<const libhttp::DefaultResponse *>(raw_response);
            //     // For HEAD requests, exclude the body
            //     bool includeBody = (request.getMethod() != libhttp::HEAD);
            //     _write_buffer = response->toString(includeBody);
            // }
            // else if (dynamic_cast<const libhttp::StreamResponse *>(raw_response))
            // {
            //     const libhttp::StreamResponse *response = dynamic_cast<const libhttp::StreamResponse *>(raw_response);
            //     _write_buffer = "";
            //     StreamSocket *ss = new StreamSocket(response->getFD(), this);
            //     el->addEvent(ss->getSocketfd(), IO_READ, ss);
            // }
            // else
            // {
            //     _write_buffer = "WTF";
            // }
        }
        */
    }

    // --- Sub-case C: Socket Ready to Write (HTTP Response) ---
    if (events & IO_WRITE)
    {
        std::cout << "WE ARE READY TO WRITE BOSS " << getHost() << std::endl;

        if (_write_buffer.empty())
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
    // STEP 2: Read data from socket into buffer
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
    // std::cout << "Try write to client " << write_buffer << std::endl;
    if (_write_buffer.empty())
    {
        return true;
    }

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

    // STEP 6: Check if all data sent
    if (_write_buffer.empty())
    {
        std::cout << "All data sent to client " << getHost() << std::endl;
        _isResponseSent = 1;
    }
    return true;
}
