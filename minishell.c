//https://github.com/sisardor/Mini-shell/blob/master/src/minishell.c


#include <stdlib.h>
#include <stdio.h>

char *cmd1[] = { "/bin/ls", "-al", "/", 0 };
char *cmd2[] = { "/usr/bin/tr", "a-z", "A-Z", 0 };

int main(int argc, char **argv)
{
	int pid, status;
	int fd[2];

	pipe(fd);

	//runsource(fd);

	switch (pid = fork())
	{

		case 0: /* child */
			dup2(fd[1], 1);	/* this end of the pipe becomes the standard output */
			close(fd[0]); 		/* this process does not need the other end */
			execvp(cmd1[0], cmd1);	/* run the command */
			fprintf(stderr,"%s failed\n",cmd1[0]);	/* commandt failed! */

		default: /* parent does nothing */
			break;

		case -1:
			fprintf(stderr, "fork failed\n");
			exit(1);
	}

	//rundest(fd);
	switch (pid = fork())
	{
		case 0: /* child */
			dup2(fd[0], 0);	/* this end of the pipe becomes the standard input */
			close(fd[1]);		/* this process doesn't need the other end */
			execvp(cmd2[0], cmd2);	/* run the command */
			fprintf(stderr,"%s failed\n", cmd2[0]);	/* it failed! */

		default: /* parent does nothing */
			break;

		case -1:
			fprintf(stderr,"fork failed\n");
			exit(1);
	}

	close(fd[0]); close(fd[1]); 	/* this is important! close both file descriptors on the pipe */

	while ((pid = wait(&status)) != -1)	/* pick up all the dead children */
		fprintf(stderr, "process %d exits with %d\n", pid, status);
	exit(0);
}
