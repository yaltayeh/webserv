#include "../include/webserv.hpp"

// ============================================================================
// Constants
// ============================================================================
static const int DEFAULT_PORT = 8080;
static const int POLL_TIMEOUT_INFINITE = -1;

// ============================================================================
// LEARNING ROADMAP - Implement in this order:
// ============================================================================
// PHASE 1: Basic Socket Server ✓ DONE
//   - Created listening socket that waits for connections
//   - Used socket(), bind(), listen() to set up TCP server
//
// PHASE 2: Non-Blocking I/O ✓ DONE
//   - All sockets set to O_NONBLOCK with fcntl()
//   - Sockets won't freeze our program waiting for data
//
// PHASE 3: I/O Multiplexing with poll() ✓ IN PROGRESS
//   - Using poll() to monitor multiple sockets at once
//   - Can accept new connections while serving existing clients
//   - Next: Implement reading/writing HTTP data
//
// PHASE 4: HTTP Protocol Integration - TODO
//   - Parse HTTP requests using libhttp::Parser
//   - Generate HTTP responses
//   - Handle different request methods (GET, POST, DELETE)
//
// PHASE 5: Configuration File Parser - TODO
//   - Parse NGINX-like config files
//   - Support multiple server blocks on different ports
//   - Handle routes, error pages, file uploads
//
// PHASE 6: CGI Execution - TODO
//   - Execute PHP/Python scripts with fork() and execve()
//   - Pass environment variables and request data
//   - Capture output and send back to client
// ============================================================================

// ============================================================================
// Initialize server listening socket
// ============================================================================
// RETURNS:
// - Socket fd on success
// - -1 on failure (program should exit)
static int initializeServer() {
	int listen_fd = createListeningSocket(DEFAULT_PORT);
	if (listen_fd < 0) {
		std::cerr << "Failed to create listening socket" << std::endl;
		return -1;
	}
	
	std::cout << "Listening on port " << DEFAULT_PORT << std::endl;
	return listen_fd;
}

// ============================================================================
// Handle listening socket activity (new connections)
// ============================================================================
// PARAMETERS:
// - listen_fd: The listening socket
// - connections: Vector of active connections
// - poll_fds: Vector of poll file descriptors
static void handleListeningSocket(int listen_fd,
                                  std::vector<Connection>& connections,
                                  std::vector<pollfd>& poll_fds,
                                  short revents) {
	if (revents & POLLIN)
		handleNewConnection(listen_fd, connections, poll_fds);
}

// ============================================================================
// Handle client socket activity (read/write/error)
// ============================================================================
// PARAMETERS:
// - index: Index in poll_fds array
// - connections: Vector of active connections
// - poll_fds: Vector of poll file descriptors
//
// RETURNS:
// - true if connection should be kept
// - false if connection should be removed
static bool handleClientSocket(size_t index,
                              std::vector<Connection>& connections,
                              std::vector<pollfd>& poll_fds) {
	Connection* conn = &connections[index - 1];  // -1: poll_fds[0] is listening socket
	pollfd* poll_fd = &poll_fds[index];
	
	// ========================================================================
	// Check for errors or connection closure first
	// ========================================================================
	if (poll_fd->revents & (POLLERR | POLLHUP))
		return handleSocketError(poll_fd->fd, poll_fd->revents);
	
	// ========================================================================
	// Handle incoming data (POLLIN)
	// ========================================================================
	if (poll_fd->revents & POLLIN) {
		if (!handleClientRead(conn, poll_fd))
			return false;
	}
	
	// ========================================================================
	// Handle outgoing data (POLLOUT)
	// ========================================================================
	if (poll_fd->revents & POLLOUT) {
		if (!handleClientWrite(conn, poll_fd))
			return false;
	}
	
	return true;
}

int main() {
	// ========================================================================
	// SETUP: Create the listening socket
	// ========================================================================
	int listen_fd = initializeServer();
	if (listen_fd < 0)
		return 1;
	
	// ========================================================================
	// SETUP: Initialize data structures
	// ========================================================================
	// connections[]: Stores Connection objects (one per client)
	// poll_fds[]:    Stores pollfd structures for poll() monitoring
	//
	// IMPORTANT: These arrays stay in sync!
	// - poll_fds[0] = listening socket (NOT in connections[])
	// - poll_fds[i] = connections[i-1].fd for i > 0
	std::vector<Connection> connections;
	std::vector<pollfd> poll_fds;
	
	addSocketToPoll(listen_fd, poll_fds, POLLIN);

	// ========================================================================
	// MAIN EVENT LOOP - The heart of the server
	// ========================================================================
	// This loop never ends (until error or shutdown signal)
	// Each iteration:
	// 1. poll() waits for activity on any socket
	// 2. We check which sockets have activity
	// 3. We handle each active socket appropriately
	while (true) {
		// ====================================================================
		// STEP 1: Wait for activity on any socket
		// ====================================================================
		// poll() monitors ALL sockets and blocks until one has activity
		//
		// PARAMETERS:
		// - &poll_fds[0]: Pointer to first element (array start)
		// - poll_fds.size(): Number of file descriptors to monitor
		// - POLL_TIMEOUT_INFINITE (-1): Wait forever, no timeout
		//
		// RETURNS:
		// - Positive: Number of sockets with activity
		// - 0: Timeout expired (won't happen with -1)
		// - -1: Error occurred
		int ready = poll(&poll_fds[0], poll_fds.size(), POLL_TIMEOUT_INFINITE);
		if (ready < 0) {
			std::cerr << "poll() error" << std::endl;
			break;
		}
		
		// ====================================================================
		// STEP 2: Process all file descriptors with activity
		// ====================================================================
		// We loop through and check each fd's revents
		// If revents == 0, nothing happened on that socket
		for (size_t i = 0; i < poll_fds.size(); i++) {
			if (poll_fds[i].revents == 0)
				continue;
			
			// Handle listening socket (accept new connections)
			if (poll_fds[i].fd == listen_fd) {
				handleListeningSocket(listen_fd, connections, poll_fds,
				                     poll_fds[i].revents);
				continue;
			}
			
			// Handle client socket (read/write/error)
			if (!handleClientSocket(i, connections, poll_fds))
				removeConnection(i, connections, poll_fds);
		}
	}
	
	// ========================================================================
	// CLEANUP: Server is shutting down
	// ========================================================================
	std::cout << "Server shutting down..." << std::endl;
	
	close(listen_fd);
	for (size_t i = 0; i < connections.size(); i++)
		close(connections[i].fd);
	
	return 0;
}