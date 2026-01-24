#include <unistd.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int set_non_block(int fd)
{
	/**
	 * Get current flags on the file descriptor
	 */
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		return -1;
	}

	/**
	 * Add O_NONBLOCK flag and set it back
	 */
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return -1;
	}

	return 0;
}

int main()
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
		return (1);

	pid_t child = fork();
	if (child < 0)
	{
		close(sv[0]);
		close(sv[1]);
		perror("fork");
		return (1);
	}
	if (child == 0)
	{
		close(sv[0]);
		dup2(sv[1], STDOUT_FILENO);
		dup2(sv[1], STDIN_FILENO);
		close(sv[1]);
		char buf[8];
		ssize_t nbytes = read(0, buf, sizeof(buf) - 1);
		if (nbytes > 0)
		{
			buf[nbytes] = '\0';
			printf("Child read data: %s\n", buf);
		}
		return (0);
	}
	close(sv[1]);

	set_non_block(sv[0]);
	int kq = kqueue();
	if (kq < 0)
	{
		close(sv[0]);
		return (0);
	}
	struct kevent kev[2];

	EV_SET(&kev[0], sv[0], EVFILT_READ,
		   EV_ADD | EV_ENABLE,
		   0, 0, NULL);
	EV_SET(&kev[1], sv[0], EVFILT_WRITE,
		   EV_ADD | EV_ENABLE,
		   0, 0, NULL);

	kevent(kq, kev, 2, NULL, 0, NULL);

	pid_t pid = -1;
	while (1)
	{
		// printf("pid: %d\n", pid);

		struct timespec timeout;
		timeout.tv_sec = 0;
		timeout.tv_nsec = 100; // 100 milliseconds

		int nready = kevent(kq, 0, 0, kev, 2, &timeout);
		if (nready <= 0)
			break;
		printf("nready: %d\n", nready);
		pause();
		// if (nready < 0)
		// {
		// 	perror("kevent");
		// 	break;
		// }
		if (kev->ident == sv[0])
		{
			if (kev->flags & EV_EOF)
			{
				printf("Socket closed\n");
				EV_SET(&kev[0], sv[0], EVFILT_READ,
					   EV_DELETE,
					   0, 0, NULL);
				EV_SET(&kev[1], sv[0], EVFILT_WRITE,
					   EV_DELETE,
					   0, 0, NULL);
				kevent(kq, kev, 2, NULL, 0, NULL);
			}
			if (kev->flags & EV_ERROR)
			{
				printf("Socket error\n");
			}
			if (kev->filter == EVFILT_READ)
			{
				char buf[128];
				ssize_t n = read(sv[0], buf, sizeof(buf));
				if (n > 0)
					write(1, buf, n);
			}
			if (kev->filter == EVFILT_WRITE)
			{
				// static int called = 0;
				// if (called == 0)
				// 	write(sv[0], "moredatadata", 12);
				// called = 1;
				printf("Socket writable\n");
			}
		}

		// if (pid > 0)
		// {
		// 	printf("Exiting loop after handling child exit\n");
		// 	break;
		// }
		// sleep(1);

		// if ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
		// {
		// 	EV_SET(&kev[0], sv[0], EVFILT_READ,
		// 		   EV_DELETE,
		// 		   0, 0, NULL);
		// 	EV_SET(&kev[1], sv[0], EVFILT_WRITE,
		// 		   EV_DELETE,
		// 		   0, 0, NULL);
		// 	kevent(kq, kev, 2, NULL, 0, NULL);
		// 	close(sv[0]);
		// 	printf("Delet events after child exit\n");
		// }
	}

	// close(sv[0]);
	close(kq);
	return (0);
}