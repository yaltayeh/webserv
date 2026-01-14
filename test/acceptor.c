

int main()
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bind(listen_fd, addr, addrlen);
	listen(listen_fd, SOMAXCONN);
}