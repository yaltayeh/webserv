#include "../include/webserv.hpp"

// ============================================================================
// Constants
// ============================================================================
static const size_t READ_BUFFER_SIZE = 4096;  // Common page size

// ============================================================================
// Handle accepting a new client connection
// ============================================================================
// WHAT THIS DOES:
// When poll() tells us the listening socket has a POLLIN event, it means
// a client is trying to connect. This function:
// 1. Accepts the connection (creates new socket)
// 2. Creates a Connection object for this client
// 3. Adds the new socket to poll monitoring
//
// WHY THIS IS A SEPARATE FUNCTION:
// - Keeps main event loop clean and readable
// - All the "accept a connection" logic is in one place
// - Easier to debug and test
//
// PARAMETERS:
// - listen_fd: The listening socket that received the connection request
// - connections: Vector where we store all active Connection objects
// - poll_fds: Vector where we store all pollfd structures for poll()
//
// RETURNS: Nothing (void function)
void handleNewConnection(int listen_fd,
                        std::vector<Connection>& connections,
                        std::vector<pollfd>& poll_fds) {
	
	// ========================================================================
	// STEP 1: Accept the connection
	// ========================================================================
	// accept() does several things:
	// - Takes a waiting connection from the listening socket's queue
	// - Creates a brand NEW socket just for this client
	// - Makes the new socket non-blocking (done in acceptConnection)
	// - Returns the new socket's file descriptor number
	//
	// IMPORTANT: The listening socket stays open and keeps listening!
	// It's like a factory that creates new sockets for each client.
	int client_fd = acceptConnection(listen_fd);
	if (client_fd < 0) {
		std::cerr << "Failed to accept connection" << std::endl;
		return;
	}
	
	std::cout << "New client connected on fd " << client_fd << std::endl;
	
	// ========================================================================
	// STEP 2: Create a Connection object for this client
	// ========================================================================
	// The Connection struct holds everything we need for this client:
	// - fd: The socket number (how we identify this client)
	// - parser: HTTP parser that accumulates data across multiple reads
	// - write_buffer: Response data waiting to be sent
	Connection new_conn;
	new_conn.fd = client_fd;
	connections.push_back(new_conn);
	
	// ========================================================================
	// STEP 3: Add the new socket to poll monitoring
	// ========================================================================
	// We need to tell poll() to watch this new socket
	// POLLIN = "notify me when this socket has data to read"
	// We'll add POLLOUT later when we have data to send back
	addSocketToPoll(client_fd, poll_fds, POLLIN);
	
	std::cout << "Now monitoring " << connections.size()
	          << " client connection(s)" << std::endl;
}


// ============================================================================
// Handle reading data from a client socket
// ============================================================================
// WHAT THIS DOES:
// When poll() tells us a client socket has a POLLIN event, it means
// the client sent us data. This function:
// 1. Reads data from the socket into a temporary buffer
// 2. Feeds the data to the HTTP parser incrementally
// 3. Checks if the parser completed the request
// 4. If complete, generates a response and prepares to send it
//
// WHY INCREMENTAL PARSING:
// - TCP is a STREAM protocol, not messages
// - Data might arrive in chunks: "GET / HT" ... "TP/1.0\r\n\r\n"
// - Parser accumulates data across multiple read() calls
// - Parser returns: 0=complete, 1=need more data, -1=error
//
// PARAMETERS:
// - conn: Pointer to the Connection object for this client
// - poll_fd: Pointer to the pollfd structure for this client
//
// RETURNS:
// - true: Connection is still alive, keep it
// - false: Connection closed or error, should be removed
bool handleClientRead(Connection* conn, pollfd* poll_fd) {
	
	// ========================================================================
	// STEP 1: Read data from socket into buffer
	// ========================================================================
	char buffer[READ_BUFFER_SIZE];
	std::memset(buffer, 0, READ_BUFFER_SIZE);
	
	ssize_t bytes_read = read(conn->fd, buffer, READ_BUFFER_SIZE);
	
	// ========================================================================
	// STEP 2: Handle read() results
	// ========================================================================
	if (bytes_read == 0) {
		std::cout << "Client fd " << conn->fd
		          << " closed connection" << std::endl;
		return false;
	}
	
	if (bytes_read < 0) {
		std::cerr << "Error reading from client fd " << conn->fd << std::endl;
		return false;
	}
	
	std::cout << "Read " << bytes_read << " bytes from client fd "
	          << conn->fd << std::endl;
	
	// ========================================================================
	// STEP 3: Feed data to HTTP parser
	// ========================================================================
	int parse_result = conn->parser.parse(buffer, bytes_read);
	
	if (parse_result == -1) {
		std::cerr << "Parse error for client fd " << conn->fd << std::endl;
		// TODO: Send 400 Bad Request response
		return false;
	}
	
	// ========================================================================
	// STEP 4: Generate response if request is complete
	// ========================================================================
	if (parse_result == 0) {
		std::cout << "Request complete for client fd " << conn->fd << std::endl;
		
		// TODO: Generate proper HTTP response based on request
		conn->write_buffer =
			"HTTP/1.0 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello, World!";
		
		// Switch to write mode
		poll_fd->events = POLLIN | POLLOUT;
		std::cout << "Response ready, waiting for socket to be writable" << std::endl;
	}
	
	// parse_result == 1 means need more data, keep waiting
	return true;
}


// ============================================================================
// Handle writing data to a client socket
// ============================================================================
// WHAT THIS DOES:
// When poll() tells us a client socket has a POLLOUT event, it means
// we can send data without blocking. This function:
// 1. Writes data from write_buffer to the socket
// 2. Removes the sent data from write_buffer
// 3. If buffer is empty, stops watching for POLLOUT (save CPU)
//
// WHY CHECK POLLOUT:
// - Socket send buffers can fill up (especially with large files)
// - write() might only send PART of our data
// - We store unsent data in write_buffer
// - We ask poll() to notify us when socket is writable again
//
// PARAMETERS:
// - conn: Pointer to the Connection object for this client
// - poll_fd: Pointer to the pollfd structure for this client
//
// RETURNS:
// - true: Connection is still alive, keep it
// - false: Connection closed or error, should be removed
bool handleClientWrite(Connection* conn, pollfd* poll_fd) {
	
	// ========================================================================
	// STEP 1: Check if we have data to send
	// ========================================================================
	if (conn->write_buffer.empty()) {
		poll_fd->events = POLLIN;
		return true;
	}
	
	// ========================================================================
	// STEP 2: Write data to socket
	// ========================================================================
	ssize_t bytes_sent = write(conn->fd,
	                           conn->write_buffer.c_str(),
	                           conn->write_buffer.size());
	
	// ========================================================================
	// STEP 3: Handle write() results
	// ========================================================================
	if (bytes_sent < 0) {
		std::cerr << "Error writing to client fd " << conn->fd << std::endl;
		return false;
	}
	
	if (bytes_sent == 0) {
		// Nothing sent - retry on next POLLOUT
		return true;
	}
	
	std::cout << "Sent " << bytes_sent << " bytes to client fd "
	          << conn->fd << std::endl;
	
	// ========================================================================
	// STEP 4: Remove sent data from buffer
	// ========================================================================
	conn->write_buffer.erase(0, bytes_sent);
	
	// ========================================================================
	// STEP 5: Check if all data sent
	// ========================================================================
	if (conn->write_buffer.empty()) {
		std::cout << "All data sent to client fd " << conn->fd << std::endl;
		poll_fd->events = POLLIN;
		// TODO: For HTTP/1.0, close connection here
	}
	
	return true;
}


// ============================================================================
// Handle socket errors and connection closure
// ============================================================================
// WHAT THIS DOES:
// When poll() sets POLLERR (error) or POLLHUP (hangup) flags, it means:
// - POLLERR: Socket has an error condition (rare, usually system issue)
// - POLLHUP: Client closed the connection (common, normal)
//
// PARAMETERS:
// - fd: The socket file descriptor that had an error
// - revents: The events that poll() detected (POLLERR, POLLHUP, etc.)
//
// RETURNS:
// - Always returns false (connection should be removed)
bool handleSocketError(int fd, short revents) {
	if (revents & POLLERR)
		std::cerr << "POLLERR on socket fd " << fd << std::endl;
	
	if (revents & POLLHUP)
		std::cout << "POLLHUP on socket fd " << fd
		          << " (client closed connection)" << std::endl;
	
	return false;
}

// ============================================================================
// Remove a connection and clean up resources
// ============================================================================
// WHAT THIS DOES:
// Handles the cleanup when a connection needs to be removed:
// 1. Closes the socket to free system resources
// 2. Removes the pollfd from poll_fds vector
// 3. Removes the Connection from connections vector
//
// PARAMETERS:
// - index: Index in poll_fds where this connection is (passed by reference)
// - connections: Vector of Connection objects
// - poll_fds: Vector of pollfd structures for poll()
//
// IMPORTANT: Index mapping
// - poll_fds[0] is the listening socket (NOT in connections)
// - poll_fds[i] maps to connections[i-1] for client connections
//
// SIDE EFFECTS:
// - Decrements index after removal to account for shifted elements
void removeConnection(size_t& index,
                     std::vector<Connection>& connections,
                     std::vector<pollfd>& poll_fds) {
	// Close socket first to free system resources
	close(poll_fds[index].fd);
	
	// Remove from vectors (poll_fds[0] is listening socket)
	poll_fds.erase(poll_fds.begin() + index);
	connections.erase(connections.begin() + (index - 1));
	
	// Decrement index to check correct next element after shift
	index--;
}
