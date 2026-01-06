#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include "../dependencies/libhttp/include/Parser.hpp"

// ============================================================================
// Connection Structure
// ============================================================================
// Holds everything we need to track for each connected client
struct Connection {
    int fd;                      // Socket file descriptor
    libhttp::Parser parser;      // HTTP parser (accumulates across reads)
    std::string write_buffer;    // Response data waiting to be sent
};

// ============================================================================
// Socket Helper Functions
// ============================================================================
int createListeningSocket(int port);
int acceptConnection(int listen_fd);
void addSocketToPoll(int fd, std::vector<pollfd>& poll_fds, short events);
Connection* findConnection(int fd, std::vector<Connection>& connections);

// ============================================================================
// Connection Handler Functions
// ============================================================================
void handleNewConnection(int listen_fd, 
                        std::vector<Connection>& connections,
                        std::vector<pollfd>& poll_fds);
bool handleClientRead(Connection* conn, pollfd* poll_fd);
bool handleClientWrite(Connection* conn, pollfd* poll_fd);
bool handleSocketError(int fd, short revents);

#endif // WEBSERV_HPP
