//Answering the fist part of the question where the parent should wait for the two child process to finish

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
pid_t pid1, pid2;
int status;
// create the first child process

if ((pid1 = fork()) < 0) {
perror("error forking first child");
return -1;

}

if (pid1 == 0) {
// first child process - execute 'ls -al'
char *args[]={"/bin/ls","-al", NULL};
execvp(args[0], args);
}
//control does not get here since execvp has replaced the program image
// again fork
if ((pid2 = fork()) < 0) {
perror("error forking second child");
return -1;
}
if (pid2 == 0) {
// second child process execute 'grep main minishell.c'
char *args[]={"/bin/grep","main", "minishell.c", NULL};
execvp(args[0], args);
}
wait(&status);
fflush(stdout);
if (pid2 > 0 && pid1 > 0) {
printf("PID of the first child: %d\n", pid1);
printf("PID of the second child: %d\n", pid2);
}
return 0;
}

//Now coming to the second part of the question the question seems little incorrect. Ideally to expect the desired output the grep pattern should be "grep minishell". Considering that the following code does the trick -

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
pid_t pid1, pid2;
int status;
int pipefds[2];
int ret;

// create a pipe where the child processes could communicate
ret = pipe(pipefds);

if (ret < 0) {
perror("error creating pipe");
return -1;
}


// create the first child process
if ((pid1 = fork()) < 0) {
perror("error forking first child");
return -1;
}
if (pid1 == 0) {
// first child process - execute 'ls -al'
char *args[]={"/bin/ls","-al", NULL};
// close the stdout of the process; redirect to the pipe write end
close(1);
dup2(pipefds[1], 1);
//close read end of pipe
close(pipefds[0]);
execvp(args[0], args);
}
//control does not get here since execvp has replaced the program image
// again fork
if ((pid2 = fork()) < 0) {
perror("error forking second child");
return -1;
}
if (pid2 == 0) {
// second child process execute 'grep main minishell.c'
char *args[]={"/bin/grep","minishell",NULL};
// redirect the stdin to the read end of pipe
close(0);
dup2(pipefds[0], 0);
//close write end of pipe
close(pipefds[1]);
execvp(args[0], args);
}
// close both pipefds in the parent since it never uses it
close (pipefds [0]);
close (pipefds [1]);
wait(&status);
fflush(stdout);
return 0;
}
