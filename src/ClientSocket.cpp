#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <RequestRouter.hpp>

#define READ_BUFFER_SIZE 4096

ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
    : Socket(CLIENT_ID, client_fd),
      addr(addr),
      listenSocket(listen),
      complateResponse(false)
{
}

ClientSocket::~ClientSocket()
{
    (void)listenSocket;
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

    int parse_result = parser.parse(buffer, bytes_read);

    if (parse_result == -1)
    {
        // TODO: Generate 400 Bad Request response instead of just closing
        std::cerr << "Parse error for client " << getHost() << std::endl;
        return false;
    }

    // STEP 5: Generate response if request is complete
    if (parse_result == 0)
    {
        std::cout << "Request complete for client " << getHost() << std::endl;

        // Route the request and generate response
        RequestRouter router;
        libhttp::Request request = parser.getRequest();
        const std::vector<VirtualServer *> &servers = listenSocket->getVirtualServer();
        if (servers.empty())
        {
            std::cerr << "Error: No virtual servers configured" << std::endl;
            return false;
        }

        const ServerConfig &server_config = servers[0]->getConfig();

        libhttp::Response response = router.route(request, server_config);

        write_buffer = response.toString();
        handleWrite();
        complateResponse = 1;
        std::cout << "Response ready, waiting for socket to be writable"
                  << std::endl;
    }

    return true;
}

bool ClientSocket::handleWrite()
{
    if (write_buffer.empty())
    {
        return true;
    }

    complateResponse = 0;

    ssize_t bytes_sent = send(write_buffer.c_str(), write_buffer.size());

    if (bytes_sent < 0)
    {
        std::cerr << "Error writing to client " << getHost() << std::endl;
        return false;
    }

    if (bytes_sent == 0)
    {
        return true;
    }

    std::cout << "Sent " << bytes_sent << " bytes to client " << getHost() << std::endl;

    write_buffer.erase(0, bytes_sent);

    // STEP 6: Check if all data sent
    if (write_buffer.empty())
    {
        std::cout << "All data sent to client " << getHost() << std::endl;
    }
    return true;
}

bool ClientSocket::isComplateResponse()
{
	return complateResponse;
}
