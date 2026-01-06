#include "../include/webserv.hpp"


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
	
	// Check if accept() failed
	// Common reasons for failure:
	// - Client disconnected before we could accept
	// - System ran out of file descriptors (too many open files)
	// - System error (rare)
	if (client_fd < 0) {
		std::cerr << "Failed to accept connection" << std::endl;
		return; // Give up on this connection, but keep server running
	}
	
	std::cout << "New client connected! Socket fd: " << client_fd << std::endl;
	
	// ========================================================================
	// STEP 2: Create a Connection object for this client
	// ========================================================================
	// The Connection struct holds everything we need for this client:
	// - fd: The socket number (how we identify this client)
	// - parser: HTTP parser that accumulates data across multiple reads
	// - write_buffer: Response data waiting to be sent
	Connection new_conn;
	new_conn.fd = client_fd;
	// Note: parser and write_buffer are automatically initialized
	// - parser starts in REQUEST_LINE state
	// - write_buffer starts as empty string
	
	// Add to our list of active connections
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
	// STEP 1: Prepare buffer for reading
	// ========================================================================
	// We need a temporary buffer to hold data from read()
	// Why 4096 bytes? It's a common page size, good balance of:
	// - Not too small (fewer system calls needed)
	// - Not too large (less wasted memory)
	const size_t BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE];
	
	// Zero out the buffer for safety
	// memset(pointer, value, size) sets every byte to 0
	std::memset(buffer, 0, BUFFER_SIZE);
	
	// ========================================================================
	// STEP 2: Read data from socket
	// ========================================================================
	// read() attempts to read up to BUFFER_SIZE bytes
	// It returns:
	// - Positive number: How many bytes were actually read
	// - 0: Client closed the connection gracefully (EOF)
	// - -1: Error occurred (check errno for details)
	//
	// IMPORTANT: read() might return LESS than BUFFER_SIZE
	// - This is normal! TCP sends data in chunks
	// - Might get 500 bytes even though we asked for 4096
	// - We must handle partial reads
	ssize_t bytes_read = read(conn->fd, buffer, BUFFER_SIZE);
	
	// ========================================================================
	// STEP 3: Handle read() results
	// ========================================================================
	if (bytes_read == 0) {
		// Client closed the connection gracefully
		// This is normal - client is done talking to us
		std::cout << "Client fd " << conn->fd << " closed connection" << std::endl;
		return false; // Tell caller to remove this connection
	}
	
	if (bytes_read < 0) {
		// Error occurred during read
		// In non-blocking mode, EAGAIN/EWOULDBLOCK means "no data right now"
		// But we only call this function when poll() says POLLIN!
		// So this is a real error
		std::cerr << "Error reading from client fd " << conn->fd << std::endl;
		return false; // Tell caller to remove this connection
	}
	
	std::cout << "Read " << bytes_read << " bytes from client fd " << conn->fd << std::endl;
	
	// ========================================================================
	// STEP 4: Feed data to HTTP parser
	// ========================================================================
	// The parser accumulates data across multiple calls
	// It processes the data and advances its internal state:
	// REQUEST_LINE → HEADERS → BODY → COMPLETE (or ERROR)
	//
	// parse() returns:
	// - 0: Request is complete! Ready to generate response
	// - 1: Need more data, keep reading
	// - -1: Parse error (malformed HTTP request)
	int parse_result = conn->parser.parse(buffer, bytes_read);
	
	if (parse_result == -1) {
		// Malformed HTTP request
		// Client sent invalid data, reject it
		std::cerr << "Parse error for client fd " << conn->fd << std::endl;
		// TODO: Send 400 Bad Request response
		return false; // Remove this connection
	}
	
	if (parse_result == 0) {
		// ====================================================================
		// Request is COMPLETE! Time to generate response
		// ====================================================================
		std::cout << "Request complete for client fd " << conn->fd << std::endl;
		
		// TODO: Generate proper HTTP response based on request
		// For now: Send a simple HTTP/1.0 response
		conn->write_buffer = 
			"HTTP/1.0 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello, World!";
		
		// ====================================================================
		// Tell poll() we want to WRITE to this socket
		// ====================================================================
		// We add POLLOUT flag to tell poll():
		// "Notify me when this socket is ready to accept data"
		// We keep POLLIN in case client sends more requests (HTTP pipelining)
		poll_fd->events = POLLIN | POLLOUT;
		
		std::cout << "Response ready, waiting for socket to be writable" << std::endl;
	}
	
	// If parse_result == 1, we need more data
	// Just return true and wait for next POLLIN event
	return true; // Keep connection alive
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
	// The write_buffer might be empty if:
	// - We already sent everything
	// - We got POLLOUT but have nothing to send (shouldn't happen)
	if (conn->write_buffer.empty()) {
		// Nothing to send, stop watching for POLLOUT
		// Only watch for POLLIN (client might send another request)
		poll_fd->events = POLLIN;
		return true; // Keep connection alive
	}
	
	// ========================================================================
	// STEP 2: Try to write data to socket
	// ========================================================================
	// write() attempts to send data from write_buffer
	// It returns:
	// - Positive number: How many bytes were actually sent
	// - 0: Nothing sent (shouldn't happen with POLLOUT)
	// - -1: Error occurred
	//
	// CRITICAL: write() might send LESS than we asked for!
	// Example: write_buffer has 10000 bytes, write() only sends 3000
	// This is called a "partial write" and is NORMAL
	// We must:
	// 1. Remove the 3000 bytes we sent from write_buffer
	// 2. Keep the remaining 7000 bytes for next write
	// 3. Wait for next POLLOUT event
	ssize_t bytes_sent = write(conn->fd, 
	                           conn->write_buffer.c_str(), 
	                           conn->write_buffer.size());
	
	// ========================================================================
	// STEP 3: Handle write() results
	// ========================================================================
	if (bytes_sent < 0) {
		// Error during write - connection probably broken
		std::cerr << "Error writing to client fd " << conn->fd << std::endl;
		return false; // Remove connection
	}
	
	if (bytes_sent == 0) {
		// Nothing sent - this is weird, but not fatal
		// Just try again on next POLLOUT
		return true; // Keep connection alive
	}
	
	std::cout << "Sent " << bytes_sent << " bytes to client fd " << conn->fd << std::endl;
	
	// ========================================================================
	// STEP 4: Remove sent data from buffer
	// ========================================================================
	// erase(start, count) removes characters from the string
	// We remove from position 0 (start) for bytes_sent characters
	// If we sent everything, buffer becomes empty
	// If we sent partial data, buffer keeps the unsent portion
	conn->write_buffer.erase(0, bytes_sent);
	
	// ========================================================================
	// STEP 5: Check if we're done sending
	// ========================================================================
	if (conn->write_buffer.empty()) {
		// All data sent! No longer need POLLOUT notifications
		std::cout << "All data sent to client fd " << conn->fd << std::endl;
		
		// Stop watching for POLLOUT, only watch for POLLIN
		// This saves CPU - poll() won't wake us up for POLLOUT anymore
		poll_fd->events = POLLIN;
		
		// TODO: For HTTP/1.0, we should close the connection now
		// For HTTP/1.1, we'd keep it alive for pipelining
	}
	
	return true; // Keep connection alive
}


// ============================================================================
// Handle socket errors and connection closure
// ============================================================================
// WHAT THIS DOES:
// When poll() sets POLLERR (error) or POLLHUP (hangup) flags, it means:
// - POLLERR: Socket has an error condition (rare, usually system issue)
// - POLLHUP: Client closed the connection (common, normal)
//
// This function just logs what happened and returns false so the
// connection gets cleaned up.
//
// WHY THIS MATTERS:
// - We need to detect when clients disconnect
// - We must close() the socket to free system resources
// - We must remove the connection from our vectors
//
// PARAMETERS:
// - fd: The socket file descriptor that had an error
// - revents: The events that poll() detected (POLLERR, POLLHUP, etc.)
//
// RETURNS:
// - Always returns false (connection should be removed)
bool handleSocketError(int fd, short revents) {
	// Check what type of event occurred
	// We use & (bitwise AND) because revents is a bitmask
	// Multiple flags can be set at once
	
	if (revents & POLLERR) {
		// Socket has an error condition
		// This is rare - usually means system problem
		std::cerr << "POLLERR on socket fd " << fd << std::endl;
	}
	
	if (revents & POLLHUP) {
		// Client closed the connection
		// This is normal - client is done talking to us
		std::cout << "POLLHUP on socket fd " << fd << " (client closed connection)" << std::endl;
	}
	
	// Tell caller to remove this connection
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
// WHY THIS IS A SEPARATE FUNCTION:
// - This cleanup pattern is repeated 3 times in main event loop
// - Centralizing it makes the code cleaner and easier to maintain
// - Ensures we always close the socket before removing from vectors
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
	// Close the socket first to free system resources
	close(poll_fds[index].fd);
	
	// Remove from poll_fds (at the given index)
	poll_fds.erase(poll_fds.begin() + index);
	
	// Remove from connections (at index-1 because poll_fds[0] is listening socket)
	connections.erase(connections.begin() + (index - 1));
	
	// Decrement index because we removed an item and shifted everything down
	// This ensures the caller's loop checks the correct next element
	index--;
}
