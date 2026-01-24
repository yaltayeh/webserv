#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>
#include "EventLoop.hpp"
#include "StreamResponse.hpp"
#include "DefaultResponse.hpp"
#include "FileResponse.hpp"
#include "StreamSocket.hpp"
#include <unistd.h>

#define READ_BUFFER_SIZE (4096 * 2)

ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
    : Socket(client_fd),
      _addr(addr),
      _listenSocket(listen),
      _write_buffer(""),
      _parser(NULL),
      _response(NULL),
      _isHeadRequest(false)
{
    _write_buffer.reserve(READ_BUFFER_SIZE);
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
        _write_buffer.reserve(READ_BUFFER_SIZE);
    _write_buffer.append(buf, len);
}

IOEventState ClientSocket::handleEvent(EventLoop *el)
{
    // --- Sub-case A: Client Disconnected ---
    if (events & (IO_ERROR | IO_HUP))
    {
        std::cout << *this << " disconnected" << std::endl;
        return IO_EVENT_CLOSE;
    }

    // --- Sub-case B: Data Ready to Read (HTTP Request) ---
    if (events & IO_READ)
    {
        libhttp::State state = handleRead();
        // std::cout << "Parser state after read: " << state << std::endl;

        if (state == libhttp::CLOSED)
            return IO_EVENT_CLOSE;

        if (state == libhttp::BODY_EXCEEDED_MAX_SIZE)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            // _response = new libhttp::DefaultResponse(413);
            return IO_EVENT_CLOSE; // TODO: fix error handling later
        }

        if (state == libhttp::ERROR)
        {
            el->deleteEvent(_socketfd, IO_READ, this);
            // _response = new libhttp::DefaultResponse(400);
            return IO_EVENT_CLOSE; // TODO: fix error handling later
        }

        if (state == libhttp::COMPLETE)
        {
            el->deleteEvent(_socketfd, IO_READ, this);

            const libhttp::Request request = _parser->getRequest();
            
            // Track if this is a HEAD request (need to suppress body)
            _isHeadRequest = (request.getMethod() == libhttp::HEAD);
            
            const VirtualServer *owner_server = VirtualServer::findMatchServer(
                _listenSocket->getVirtualServers(),
                request);

            _response = owner_server->handleRequest(request);

            if (dynamic_cast<StreamResponse *>(_response))
            {
                StreamResponse *response = dynamic_cast<StreamResponse *>(_response);
                StreamSocket *ss = response->getStreamSocket();
                _write_buffer = "";
                el->addEvent(ss->getSocketfd(), IO_READ | IO_WRITE, ss);
                el->addChildProcess(ss);
            }
        }
    }

    // --- Sub-case C: Socket Ready to Write (HTTP Response) ---
    if (events & IO_WRITE)
    {
        // Send HTTP response data
        handleWrite();

        // std::cout << "WE ARE READY TO WRITE BOSS " << *this << std::endl;

        // If entire response has been sent
        if (_write_buffer.empty() && _response && _response->isComplete())
            return IO_EVENT_CLOSE;
    }
    return IO_EVENT_CONTINUE;
}

void ClientSocket::handleDestroy(EventLoop *el)
{
    el->deleteEvent(_socketfd, IO_READ | IO_WRITE, this);
    el->clientDisconnect(this);
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
    if (_response && _response->hasChunk())
    {
        char chunk_buffer[READ_BUFFER_SIZE];
        size_t chunk_size = _response->genChunk(chunk_buffer, sizeof(chunk_buffer));
        WriteToBuffer(chunk_buffer, chunk_size);
        
        // For HEAD requests, only send up to end of headers (including \r\n\r\n)
        if (_isHeadRequest)
        {
            // Find the end of headers marker
            size_t header_end = _write_buffer.find("\r\n\r\n");
            if (header_end != std::string::npos)
            {
                // Keep headers + the \r\n\r\n, discard body
                _write_buffer.erase(header_end + 4);
                // Consume all remaining chunks from response
                while (_response->hasChunk())
                {
                    _response->genChunk(chunk_buffer, sizeof(chunk_buffer));
                }
            }
        }
    }

    if (_write_buffer.empty())
        return 0;

    ssize_t bytes_sent = send(_write_buffer.c_str(), _write_buffer.size());

    if (bytes_sent < 0)
    {
        std::cerr << "Error writing to client " << *this << std::endl;
        return 0;
    }

    if (bytes_sent == 0)
        return 1;

    // std::cout << "Sent " << bytes_sent << " bytes to client " << *this << std::endl;

    _write_buffer.erase(0, bytes_sent);

    // Check if all data sent
    if (_write_buffer.empty() && _response->isComplete())
    {
        std::cout << "All data sent to client " << *this << std::endl;
    }
    return 1;
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
