#include "ClientSocket.hpp"
#include <arpa/inet.h>
#include <iostream>

#define READ_BUFFER_SIZE 4096

ClientSocket::ClientSocket(int client_fd, sockaddr_in addr, ListenSocket *listen)
	: Socket(CLIENT_ID, client_fd), addr(addr), listenSocket(listen)
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

bool ClientSocket::handleRead()
{
    char buffer[READ_BUFFER_SIZE];
    
    ssize_t bytes_read = recv(buffer, sizeof(buffer));

    if (bytes_read == 0) {
        std::cout << "Client " << getHost() << " closed connection" << std::endl;
        return false;
    }

    if (bytes_read < 0) {
        std::cerr << "Error reading from client " << getHost() << std::endl;
        return false;
    }

    std::cout << "Read " << bytes_read << " bytes from client " << getHost() << std::endl;
    
    int parse_result = parser.parse(buffer, bytes_read);
    
    if (parse_result == -1) {
        std::cerr << "Parse error for client " << getHost() << std::endl;
        return false;
    }

    if (parse_result == 0) {
        std::cout << "Request complete for client " << getHost() << std::endl;
        
        write_buffer =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";
        
        std::cout << "Response ready, waiting for socket to be writable" << std::endl;
    }
    return true;
}

bool ClientSocket::handleWrite()
{
    
    if (write_buffer.empty()) {
        return true;
    }
    
    ssize_t bytes_sent = send(write_buffer.c_str(), write_buffer.size());

    if (bytes_sent < 0) {
        std::cerr << "Error writing to client " << getHost() << std::endl;
        return false;
    }
    
    if (bytes_sent == 0) {
        return true;
    }
    
    std::cout << "Sent " << bytes_sent << " bytes to client fd "
              << getHost() << std::endl;

    write_buffer.erase(0, bytes_sent);
    
    if (write_buffer.empty()) {
        std::cout << "All data sent to client " << getHost() << std::endl;
    }
    return true;
}

