#include "../include/webserv.hpp"

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


int main() {
	// ========================================================================
	// SETUP: Create the listening socket
	// ========================================================================
	// This socket is special - it only accepts NEW connections
	// It never sends or receives HTTP data
	// Each accepted connection gets its own NEW socket
	int listen_fd = createListeningSocket(8080);
	if (listen_fd < 0) {
		std::cerr << "Failed to create listening socket" << std::endl;
	    return 1;
	}
	std::cout << "Listening on port 8080" << std::endl;

	// ========================================================================
	// SETUP: Initialize data structures
	// ========================================================================
	// connections[]: Stores Connection objects (one per client)
	//                Each has: socket fd, HTTP parser, write buffer
	//
	// poll_fds[]:    Stores pollfd structures for poll() monitoring
	//                Each has: socket fd, events to watch, events that happened
	//
	// IMPORTANT: These arrays stay in sync!
	// - poll_fds[0] = listening socket (NOT in connections[])
	// - poll_fds[1] = connections[0].fd
	// - poll_fds[2] = connections[1].fd
	// - etc.
	std::vector<Connection> connections;
	std::vector<pollfd> poll_fds;
	
	// Add listening socket to poll monitoring
	// POLLIN means "notify me when someone tries to connect"
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
		// poll() monitors ALL sockets at once and BLOCKS until one has activity
		// 
		// WHAT poll() IS WATCHING:
		// - First iteration: ONLY the listening socket (poll_fds has 1 item)
		// - After clients connect: listening socket + all client sockets
		// - poll_fds grows as we accept connections (we push new pollfd structs)
		//
		// HOW poll() WORKS:
		// - We give it an array of pollfd structures
		// - Each pollfd says: "watch this fd for these events"
		// - poll() sleeps until at least one event happens
		// - poll() fills in the revents field for each fd
		// - We loop through and check revents to see what happened
		//
		// PARAMETERS EXPLAINED:
		// - &poll_fds[0]: Pointer to first element of vector (array start)
		// - poll_fds.size(): Number of file descriptors to monitor
		// - -1: Timeout in milliseconds (-1 = wait forever, no timeout)
		//
		// RETURN VALUE:
		// - Positive number: How many sockets have activity
		// - 0: Timeout expired (won't happen with -1)
		// - -1: Error occurred
		int ready = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ready < 0) {
		    std::cerr << "poll() error" << std::endl;
		    break;
		}
		
		// ====================================================================
		// STEP 2: Loop through ALL file descriptors to see which ones are ready
		// ====================================================================
		// poll() told us SOMETHING happened, but we don't know what.
		// We must check each fd's revents field to see if it has activity.
		// 
		// ABOUT THE LOOP DIRECTION AND REMOVAL:
		// - This loop goes FORWARD (i++), which normally causes problems when removing
		// - Problem: If we remove items during iteration, indices shift
		// - Solution: After removing an item with erase(), we decrement i (i--)
		// - This makes i point to the same logical position after the shift
		// - Example: Removing item at i=2 shifts everything down, so i-- makes us check the new item at position 2
		//
		// WHAT WE'RE CHECKING:
		// - poll_fds[i].revents: Events that actually happened (filled by poll)
		// - If revents == 0: Nothing happened on this fd, skip it
		// - If revents != 0: Something happened, figure out what
		for (size_t i = 0; i < poll_fds.size(); i++) {
			
			// ================================================================
			// STEP 2.1: Skip file descriptors with no activity
			// ================================================================
			// poll() sets revents to 0 if nothing happened on this socket
			// We only care about sockets that have activity, so skip the rest
			if (poll_fds[i].revents == 0) { continue; }
			
			// ================================================================
			// STEP 2.2: Handle the LISTENING socket
			// ================================================================
			// The listening socket is special - it's the only socket that:
			// - Accepts NEW connections (creates new sockets)
			// - Never sends or receives HTTP data
			// - Is always at poll_fds[0] (first item)
			//
			// HOW WE KNOW IT'S THE LISTENING SOCKET:
			// - Compare fd number: poll_fds[i].fd == listen_fd
			// - If they match, this is the listening socket
			if (poll_fds[i].fd == listen_fd) {
				// ============================================================
				// Check if there's an incoming connection waiting
				// ============================================================
				// POLLIN on a listening socket means: "client trying to connect"
				// The & operator checks if the POLLIN bit is set in revents
				// (revents is a bitmask - multiple flags can be set at once)
				if (poll_fds[i].revents & POLLIN) {
					// Call our helper function to accept the connection
					// This will create a new socket, add Connection object,
					// and add to poll monitoring
					handleNewConnection(listen_fd, connections, poll_fds);
				}
				// Done handling listening socket, check next fd
				continue;
			}
			
			// ================================================================
			// STEP 2.3: Handle CLIENT sockets (not the listening socket)
			// ================================================================
			// If we reach here, this is a regular client connection
			// It can have several types of activity:
			// - POLLIN: Client sent us data (HTTP request)
			// - POLLOUT: Socket is ready for us to send data (HTTP response)
			// - POLLERR: Socket has an error
			// - POLLHUP: Client closed the connection
			
			// Find the Connection object for this socket
			// We need this to access the parser and write_buffer
			Connection* conn = &connections[i - 1]; // -1 because poll_fds[0] is listening socket
			
			// ============================================================
			// Check for errors or connection closure
			// ============================================================
			// POLLERR: Socket error (rare, usually system issue)
			// POLLHUP: Client closed connection (common, normal)
			// We check these FIRST before trying to read/write
			if (poll_fds[i].revents & (POLLERR | POLLHUP)) {
				// Handle the error/closure and check if we should remove connection
				if (!handleSocketError(poll_fds[i].fd, poll_fds[i].revents))
					removeConnection(i, connections, poll_fds);
				continue;
			}
			
			// ============================================================
			// Handle incoming data from client (POLLIN)
			// ============================================================
			// POLLIN means: "socket has data ready to read"
			// - read() will not block
			// - Data might be incomplete (need to accumulate)
			// - Feed to HTTP parser incrementally
			if (poll_fds[i].revents & POLLIN) {
				// Read data, parse HTTP request, generate response if complete
				if (!handleClientRead(conn, &poll_fds[i])) {
					// Connection closed or error during read
					removeConnection(i, connections, poll_fds);
					continue;
				}
			}
			
			// ============================================================
			// Handle sending data to client (POLLOUT)
			// ============================================================
			// POLLOUT means: "socket is ready to accept data"
			// - write() will not block
			// - Might only send part of our buffer (partial write)
			// - Keep writing until buffer is empty
			if (poll_fds[i].revents & POLLOUT) {
				// Send data from write_buffer to socket
				if (!handleClientWrite(conn, &poll_fds[i])) {
					// Connection closed or error during write
					removeConnection(i, connections, poll_fds);
					continue;
				}
			}
		}
	}
	
	// ========================================================================
	// CLEANUP: Server is shutting down
	// ========================================================================
	// Close all sockets to free system resources
	// This only happens if we break out of the main loop (error)
	std::cout << "Server shutting down..." << std::endl;
	close(listen_fd);
	for (size_t i = 0; i < connections.size(); i++) {
		close(connections[i].fd);
	}
	
	return 0;
}