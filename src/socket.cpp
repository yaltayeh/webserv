#include <sys/socket.h>   // socket(), setsockopt(), bind(), listen()
#include <netinet/in.h>   // sockaddr_in, htons()
#include <arpa/inet.h>    // inet_addr() if needed
#include <fcntl.h>        // fcntl() for non-blocking
#include <unistd.h>       // close()
#include <cstring>        // memset()
#include <cerrno>         // errno
#include <poll.h>		 // poll()
#include <vector>         // std::vector

int createListeningSocket(int port) {
    // 1. Create a TCP socket
    // AF_INET = IPv4, SOCK_STREAM = TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        return -1; // failed to create socket

    // 2. Allow address reuse (avoids "address already in use" errors)
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sockfd); // clean up
        return -1;
    }

    // 3. Prepare the address structure
    sockaddr_in addr;          // IPv4 address struct
    std::memset(&addr, 0, sizeof(addr)); // zero out memory
    addr.sin_family = AF_INET;            // IPv4
    addr.sin_port = htons(port);          // convert port to network byte order
    addr.sin_addr.s_addr = INADDR_ANY;    // listen on all network interfaces

    // 4. Bind the socket to our address and port
    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd); // clean up on error
        return -1;
    }

    // 5. Start listening for incoming connections
    // 128 = maximum number of waiting connections
    if (listen(sockfd, 128) < 0) {
        close(sockfd);
        return -1;
    }

    // 6. Make the socket non-blocking
    // Non-blocking means socket calls won't wait forever
    int flags = fcntl(sockfd, F_GETFL, 0); // get current flags
    if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(sockfd);
        return -1;
    }

    // 7. Return the socket file descriptor
    // This is what you'll use for accept() and other operations
    return sockfd;
}

// Read: "Beej's Guide to Network Programming" - Socket section


int acceptConnection(int listen_fd) {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) return -1;
    fcntl(client_fd, F_SETFL, O_NONBLOCK); // CRITICAL: Make it non-blocking
    return client_fd;
}

// ============================================================================
// Add a socket to the poll monitoring array
// ============================================================================
// WHY WE NEED THIS:
// poll() needs a list of sockets to monitor. We create a pollfd structure
// for each socket we want to watch, specifying what events we care about.
//
// PARAMETERS:
// - fd: The socket file descriptor to monitor
// - poll_fds: Reference to the vector where we store all pollfd structures
// - events: What to watch for (POLLIN = readable, POLLOUT = writable)
//
// WHAT THIS DOES:
// 1. Creates a new pollfd structure
// 2. Sets the fd (socket number)
// 3. Sets events (what we want to be notified about)
// 4. Initializes revents to 0 (poll() will fill this later)
// 5. Adds it to our vector so poll() sees it next iteration
void addSocketToPoll(int fd, std::vector<pollfd>& poll_fds, short events) {
	pollfd new_pollfd;
	new_pollfd.fd = fd;           // Which socket to watch
	new_pollfd.events = events;   // What events to watch for
	new_pollfd.revents = 0;       // Will be filled by poll()
	poll_fds.push_back(new_pollfd);
}