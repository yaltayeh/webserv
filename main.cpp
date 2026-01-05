#include "Response.hpp"
#include "Request.hpp"


#include <sys/socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

std::string renderResponse()
{
	Response res(""
				 "\
<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
	<meta charset=\"UTF-8\">\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	<title>Hello World</title>\
</head>\
<body>\
	<h1>Hello World</h1>\
</body>\
</html>\
"
				 "");

	return (res.getResponse());
}

int open_socket(const char *protoname)
{
	struct protoent *protoent;
	int enable = 1;

	// get port info from port name like (tcp)
	protoent = getprotobyname(protoname);
	if (protoent == NULL)
	{
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}

	int server_sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
	if (server_sockfd == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// convert socket to NON-Blocking
	// int flags = fcntl(server_sockfd, F_GETFL, 0);
	// if (flags == -1)
	// 	return -1;
	// fcntl(server_sockfd, F_SETFL, flags | O_NONBLOCK);

	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
	{
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}
	return (server_sockfd);
}

void start_listen(int server_sockfd)
{
	struct sockaddr_in server_address;
	unsigned short server_port = 12345u;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);

	if (bind(
			server_sockfd,
			(struct sockaddr *)&server_address,
			sizeof(server_address)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_sockfd, 100) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "listening on port %d\n", server_port);
}

int start_recive(int server_sockfd)
{
	socklen_t client_len;
	struct sockaddr_in client_address;

	struct pollfd fds[MAX_CLIENTS + 1];

	fds[0].fd = server_sockfd;
	fds[0].events = POLLIN;
	for (int i = 1; i <= MAX_CLIENTS; i++)
		fds[i].fd = -1;

	const std::string &response = renderResponse();

	while (1)
	{
		int ready = poll(fds, MAX_CLIENTS + 1, -1);

		if (ready < 0)
		{
			perror("poll");
			break;
		}

		if (fds[0].revents & POLLIN)
		{
			client_len = sizeof(client_address);
			int client_fd = accept(server_sockfd,
								   (struct sockaddr *)&client_address,
								   &client_len);

			for (int i = 1; i <= MAX_CLIENTS; i++)
			{
				if (fds[i].fd == -1)
				{
					fds[i].fd = client_fd;
					fds[i].events = POLLIN;
					break;
				}
			}
		}
		
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (fds[i].fd != -1 && (fds[i].revents & POLLIN))
			{
				char buffer[BUFFER_SIZE];
				ssize_t nbytes = read(fds[i].fd, buffer, sizeof(buffer));

				write(STDOUT_FILENO, buffer, nbytes);

				if (nbytes <= 0)
				{
					close(fds[i].fd);
					fds[i].fd = -1;
				}
				else
				{
					write(STDOUT_FILENO, response.c_str(), response.length()); // echo
					write(fds[i].fd, response.c_str(), response.length());	   // echo
				}
			}
		}
		printf("END REQUESTS\n");
	}
	return (0);
}

int main()
{
	// char protoname[] = "tcp";

	// int server_sockfd = open_socket(protoname);

	// start_listen(server_sockfd);

	// start_recive(server_sockfd);

	// return EXIT_SUCCESS;


	char request[] = """\
POST http/1.1\
\
""";

	Request req = Request::parseRequest(request);

}