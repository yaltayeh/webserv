#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>
#include "EventLoop.hpp"
#include "StaticResponse.hpp"
#include "StreamResponse.hpp"
#include "StreamSocket.hpp"

#define READ_BUFFER_SIZE 4096

ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
    : Socket(client_fd),
      addr(addr),
      listenSocket(listen),
      parser(NULL),
      isRequestRead(false),
      isResponseSent(false)
{
    // Initialize parser with config from the first virtual server
    const std::vector<VirtualServer *> &servers = listenSocket->getVirtualServer();
    if (!servers.empty()) {
        const ServerConfig &config = servers[0]->getConfig();
        // Create new parser with config
        parser = new libhttp::Parser(config.client_max_body_size, config.client_body_temp_path);
    } else {
        // Use default parser
        parser = new libhttp::Parser();
    }
}

ClientSocket::~ClientSocket()
{
    delete parser;
    (void)listenSocket;
}

int ClientSocket::EventHandler(EventLoop *el)
{
    // --- Sub-case A: Client Disconnected ---
    if (isEOF)
    {
        std::cout << "client [" << getHost() << "]: disconnected" << std::endl;

        // Clean up: remove from event loop and free memory
        el->deleteEvent(_socketfd, EVENT_READ);
        el->deleteEvent(_socketfd, EVENT_WRITE);
        delete this;
        return 0;
    }

    // --- Sub-case B: Data Ready to Read (HTTP Request) ---
    if (isRead)
    {
        // Read data and parse HTTP request
        if (handleRead() == false) // Returns false on error
        {
            // Error occurred, close connection
            el->deleteEvent(_socketfd, EVENT_WRITE);
            el->deleteEvent(_socketfd, EVENT_READ);
            delete this;
            return (0);
        }

        // If HTTP request is complete and response is ready
        if (isRequestRead)
        {
            // Switch from reading to writing
            // (Stop monitoring reads, start monitoring writes)
            el->deleteEvent(_socketfd, EVENT_READ);
            el->addEvent(_socketfd, EVENT_WRITE, this);

            // Route the request and generate response
            libhttp::Request request = parser->getRequest();
            const std::vector<VirtualServer *> &servers = listenSocket->getVirtualServer();
            if (servers.empty())
            {
                std::cerr << "Error: No virtual servers configured" << std::endl;
                return false;
            }

            // TODO: assign virtual server function
            // TODO: turn the request ownership to the selected virtual server
            ;

            const VirtualServer *server = VirtualServer::findMatchServer(servers, request);
            if (!server)
            {
                std::cerr << "Virtual Server Not Found" << std::endl;
                return (0);
            }
            const libhttp::Response *raw_response = &server->handleRequest(request);
            if (dynamic_cast<const libhttp::StaticResponse *>(raw_response))
            {
                const libhttp::StaticResponse *response = dynamic_cast<const libhttp::StaticResponse *>(raw_response);
                write_buffer = response->toString();
            }
            else if (dynamic_cast<const libhttp::StreamResponse *>(raw_response))
            {
                const libhttp::StreamResponse *response = dynamic_cast<const libhttp::StreamResponse *>(raw_response);
                write_buffer = "";
                StreamSocket *ss = new StreamSocket(response->getFD(), this);
                el->addEvent(ss->getSocketfd(), EVENT_READ, ss);
            }
            else
            {
                write_buffer = "WTF";
            }
        }
    }

    // --- Sub-case C: Socket Ready to Write (HTTP Response) ---
    else if (isWrite)
    {
        if (write_buffer.empty())
        {
            el->deleteEvent(_socketfd, EVENT_WRITE);
            return (1);
        }
        // Send HTTP response data
        handleWrite();

        // If entire response has been sent
        if (isResponseSent)
        {
            // Clean up: close connection and free memory
            el->deleteEvent(_socketfd, EVENT_WRITE);
            el->deleteEvent(_socketfd, EVENT_READ);
            delete this; // defualt close _socketfd
        }
    }
    return (0);
}

sockaddr_in ClientSocket::getAddress()
{
    return addr;
}

const char *ClientSocket::getHost()
{
    char *addr_str = inet_ntoa(addr.sin_addr);
    return addr_str;
}

void ClientSocket::WriteToBuffer(char *buf, size_t len)
{
    if (write_buffer.capacity() <= write_buffer.size() + len)
        write_buffer.reserve(4096);
    write_buffer.append(buf, len);
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
bool ClientSocket::handleRead()
{
    // STEP 2: Read data from socket into buffer
    char buffer[READ_BUFFER_SIZE];

    ssize_t bytes_read = recv(buffer, READ_BUFFER_SIZE);

    if (bytes_read == 0)
    {
        std::cout << "Client " << getHost() << " closed connection" << std::endl;
        return false;
    }

    if (bytes_read < 0)
    {
        std::cerr << "Error reading from client " << getHost() << std::endl;
        return false;
    }

    std::cout << "Read " << bytes_read << " bytes from client " << getHost() << std::endl;

    int parse_result = parser->parse(buffer, bytes_read);

    if (parse_result == -1)
    {
        // TODO: Generate 400 Bad Request response instead of just closing
        std::cerr << "Parse error for client " << getHost() << std::endl;
        return false;
    }

    // STEP 5: Generate response if request is complete
    if (parse_result == 0)
    {
        isRequestRead = 1;
        std::cout << "Response ready, waiting for socket to be writable"
                  << std::endl;
    }

    return true;
}

bool ClientSocket::handleWrite()
{
    // std::cout << "Try write to client " << write_buffer << std::endl;
    if (write_buffer.empty())
    {
        return true;
    }

    ssize_t bytes_sent = send(write_buffer.c_str(), write_buffer.size());

    if (bytes_sent < 0)
    {
        std::cerr << "Error writing to client " << getHost() << std::endl;
        return false;
    }

    if (bytes_sent == 0)
        return true;

    std::cout << "Sent " << bytes_sent << " bytes to client " << getHost() << std::endl;

    write_buffer.erase(0, bytes_sent);

    // STEP 6: Check if all data sent
    if (write_buffer.empty())
    {
        std::cout << "All data sent to client " << getHost() << std::endl;
        isResponseSent = 1;
    }
    return true;
}
