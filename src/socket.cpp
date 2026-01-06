#include <sys/socket.h>   // socket(), setsockopt(), bind(), listen()
#include <netinet/in.h>   // sockaddr_in, htons()
#include <arpa/inet.h>    // inet_addr() if needed
#include <fcntl.h>        // fcntl() for non-blocking
#include <unistd.h>       // close()
#include <cstring>        // memset()
#include <cerrno>         // errno
#include <poll.h>         // poll()
#include <vector>         // std::vector

// ============================================================================
// Constants
// ============================================================================
static const int LISTEN_BACKLOG = 128;  // Max queued connections
static const int SOCKET_ENABLE = 1;      // Enable socket option

// ============================================================================
// Make a socket non-blocking
// ============================================================================
// WHY THIS IS CRITICAL:
// - Non-blocking sockets never freeze the program
// - read() returns immediately even if no data available
// - write() returns immediately even if buffer is full
// - accept() returns immediately even if no connection waiting
//
// RETURNS:
// - 0 on success
// - -1 on failure
static int setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;
	
	return 0;
}

// ============================================================================
// Configure socket options (reuse address)
// ============================================================================
// WHY WE NEED SO_REUSEADDR:
// - Without it, restarting server gives "Address already in use" error
// - Allows immediate rebinding after server crash/restart
// - Standard practice for development servers
//
// RETURNS:
// - 0 on success
// - -1 on failure
static int configureSocketOptions(int sockfd) {
	int opt = SOCKET_ENABLE;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return -1;
	return 0;
}

// ============================================================================
// Prepare and bind socket address
// ============================================================================
// WHAT THIS DOES:
// - Creates the address structure (IP + port)
// - Binds socket to the address so it can receive connections
//
// PARAMETERS:
// - sockfd: Socket to bind
// - port: Port number to listen on
//
// RETURNS:
// - 0 on success
// - -1 on failure
static int bindSocket(int sockfd, int port) {
	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
		return -1;
	
	return 0;
}

int createListeningSocket(int port) {
	// ========================================================================
	// STEP 1: Create a TCP socket
	// ========================================================================
	// AF_INET = IPv4, SOCK_STREAM = TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;
	
	// ========================================================================
	// STEP 2: Configure socket options (SO_REUSEADDR)
	// ========================================================================
	if (configureSocketOptions(sockfd) < 0) {
		close(sockfd);
		return -1;
	}
	
	// ========================================================================
	// STEP 3: Bind socket to address and port
	// ========================================================================
	if (bindSocket(sockfd, port) < 0) {
		close(sockfd);
		return -1;
	}
	
	// ========================================================================
	// STEP 4: Start listening for connections
	// ========================================================================
	if (listen(sockfd, LISTEN_BACKLOG) < 0) {
		close(sockfd);
		return -1;
	}
	
	// ========================================================================
	// STEP 5: Make socket non-blocking
	// ========================================================================
	if (setNonBlocking(sockfd) < 0) {
		close(sockfd);
		return -1;
	}
	
	return sockfd;
}

// ============================================================================
// Accept a new client connection
// ============================================================================
// WHAT THIS DOES:
// - Takes a waiting connection from the listening socket's queue
// - Creates a new socket for this specific client
// - Makes the new socket non-blocking
//
// PARAMETERS:
// - listen_fd: The listening socket
//
// RETURNS:
// - New socket fd on success
// - -1 on failure
int acceptConnection(int listen_fd) {
	sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	
	// Accept creates a NEW socket for this client
	int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0)
		return -1;
	
	// CRITICAL: Make new socket non-blocking
	if (setNonBlocking(client_fd) < 0) {
		close(client_fd);
		return -1;
	}
	
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
	std::memset(&new_pollfd, 0, sizeof(new_pollfd));
	
	new_pollfd.fd = fd;           // Which socket to watch
	new_pollfd.events = events;   // What events to watch for
	new_pollfd.revents = 0;       // Will be filled by poll()
	
	poll_fds.push_back(new_pollfd);
}