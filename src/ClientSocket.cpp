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
      _write_buffer(""),
      _parser(NULL),
      _response(NULL)
{
    _write_buffer.reserve(4096);
    // Initialize parser with config from the first virtual server
    const std::vector<VirtualServer *> &servers = _listenSocket->getVirtualServers();
    const ServerConfig &config = servers[0]->getConfig();

    _parser = new libhttp::Parser(config.client_max_body_size, config.client_body_temp_path);
}

ClientSocket::~ClientSocket()
{
    delete _parser;
    delete _response;
    (void)_listenSocket;
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
        el->clientDisconnect(this);
        return 0;
    }

    // --- Sub-case B: Data Ready to Read (HTTP Request) ---
    if (events & IO_READ)
    {
        libhttp::State state = handleRead();
        // std::cout << "Parser state after read: " << state << std::endl;

        if (state == libhttp::CLOSED)
        {
            el->clientDisconnect(this);
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
                request);

            _response = owner_server->handleRequest(request);

            // const libhttp::Response *raw_response = owner_server->handleRequest(request);

            // if (dynamic_cast<const libhttp::DefaultResponse *>(raw_response))
            // {
            //     const libhttp::DefaultResponse *response = dynamic_cast<const libhttp::DefaultResponse *>(raw_response);
            //     _write_buffer = response->toString();
            // }

            // else if (dynamic_cast<const libhttp::StreamResponse *>(raw_response))
            // {
            //     const libhttp::StreamResponse *response = dynamic_cast<const libhttp::StreamResponse *>(raw_response);
            //     _write_buffer = "";
            //     StreamSocket *ss = new StreamSocket(response->getFd(), this);
            //     el->addEvent(ss->getSocketfd(), IO_READ, ss);
            // }

            // else if (dynamic_cast<const libhttp::FileResponse *>(raw_response))
            // {
            //     const libhttp::FileResponse *response = dynamic_cast<const libhttp::FileResponse *>(raw_response);
            //     _write_buffer = response->toString();
            //     _file_fd = response->getFd();
            // }

            // delete raw_response;
        }
    }

    // --- Sub-case C: Socket Ready to Write (HTTP Response) ---
    if (events & IO_WRITE)
    {
        std::cout << "WE ARE READY TO WRITE BOSS " << *this << std::endl;

        // Send HTTP response data
        handleWrite();

        // If entire response has been sent
        if (_write_buffer.empty() && _response && _response->isComplete())
        {
            // Clean up: close connection and free memory
            el->clientDisconnect(this);
            return 0;
        }
    }
    return 0;
}

/**
 * handleRead() - Handles readable client socket
 *
 * Reads data from socket, feeds it to the HTTP parser, and generates
 * a response when the request is complete.
 *
 * PARAMETERS:
 * - fd: File descriptor to read from
 * - poll_fd: Pointer to corresponding pollfd struct (for modifying events)
 *
 * RETURNS:
 * - true if connection should remain open
 * - false if connection should be closed
 */

libhttp::State ClientSocket::handleRead()
{
    char buffer[READ_BUFFER_SIZE];
    ssize_t bytes_read = recv(buffer, READ_BUFFER_SIZE);

    if (bytes_read <= 0)
    {
        std::cout << "Client " << *this << " closed connection" << std::endl;
        return libhttp::CLOSED;
    }

    std::cout << *this << "Read " << bytes_read << " bytes" << std::endl;
    return _parser->parse(buffer, bytes_read);
}

bool ClientSocket::handleWrite()
{
    if (_response && _response->isHasChunk())
    {
        char chunk_buffer[READ_BUFFER_SIZE];
        size_t chunk_size = _response->genChunk(chunk_buffer, sizeof(chunk_buffer));
        WriteToBuffer(chunk_buffer, chunk_size);
    }

    if (_write_buffer.empty())
        return true;

    ssize_t bytes_sent = send(_write_buffer.c_str(), _write_buffer.size());

    if (bytes_sent < 0)
    {
        std::cerr << "Error writing to client " << *this << std::endl;
        return false;
    }

    if (bytes_sent == 0)
        return true;

    std::cout << "Sent " << bytes_sent << " bytes to client " << *this << std::endl;

    _write_buffer.erase(0, bytes_sent);

    // Check if all data sent
    if (_write_buffer.empty() && _response->isComplete())
    {
        std::cout << "All data sent to client " << *this << std::endl;
    }
    return true;
}

sockaddr_in ClientSocket::getAddress() const
{
    return _addr;
}

const char *ClientSocket::getHost() const
{
    char *addr_str = inet_ntoa(_addr.sin_addr);
    return addr_str;
}

int ClientSocket::getPort() const
{
    return ntohs(_addr.sin_port);
}

std::ostream &operator<<(std::ostream &os, const ClientSocket &client)
{
    os << "ClientSocket(fd=" << client.getSocketfd()
       << ", addr=" << client.getHost()
       << ":" << client.getPort() << ")";
    return os;
}
