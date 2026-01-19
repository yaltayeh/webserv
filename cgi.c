#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
	printf("HTTP/1.0 200\r\n\r\n");

	printf("\n-------- ARGS --------\n");
	fprintf(stderr, "\n-------- ARGS --------\n");
	for (int i = 0; i < argc; i++)
	{
		printf("arg[%d]: %s\n", i, argv[i]);
		fprintf(stderr, "arg[%d]: %s\n", i, argv[i]);
	}
	printf("\n-------- ENV --------\n");
	fprintf(stderr, "\n-------- ENV --------\n");
	for (int i = 0; envp[i]; i++)
	{
		printf("export %s\n", envp[i]);
		fprintf(stderr, "export %s\n", envp[i]);
	}
	printf("\n-------- FILE  --------\n");
	fprintf(stderr, "\n-------- FILE  --------\n");
	ssize_t nbytes;
	char buf[1024];
	while ((nbytes = read(0, buf, sizeof(buf))) > 0)
	{
		write(1, buf, nbytes);
		write(2, buf, nbytes);
	}
	return (0);
}