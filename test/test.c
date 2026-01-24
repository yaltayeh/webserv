#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// localhost:8080/bigfile_100mb.bin
int main() {
	const char *host = "localhost";
	const int port = 8080;
	
	// Create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return 1;
	}
	
	// Get host info
	struct hostent *server = gethostbyname(host);
	if (server == NULL) {
		perror("gethostbyname");
		return 1;
	}
	
	// Connect to server
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
	
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		return 1;
	}
	
	// Send HTTP request
	const char *request = "GET /bigfile_100mb.bin HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
	if (send(sock, request, strlen(request), 0) < 0) {
		perror("send");
		return 1;
	}
	
	// Receive and print response
	char buffer[4096];
	size_t total_bytes = 0;
	while (1) {
		int n = recv(sock, buffer, sizeof(buffer), 0);
		if (n <= 0) break;
		total_bytes += n;
		// Print total bytes received in human-readable format
		if (total_bytes > 1024 * 1024 * 1024)
			printf("%zu G\n", total_bytes / (1024 * 1024 * 1024));
		else if (total_bytes > 1024 * 1024)
			printf("%zu M\n", total_bytes / (1024 * 1024));
		else if (total_bytes > 1024)
			printf("%zu K\n", total_bytes / 1024);
		else
			printf("%zu B\n", total_bytes);
		// printf("%.*s", n, buffer);
	}
	
	close(sock);
	return 0;
}