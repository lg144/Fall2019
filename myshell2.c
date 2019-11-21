/*
 * Name: 
 * PID: 
 *
 * I affirm that I wrote this program myself without any help
 * from anyone or sources on the internet.
 *
 * Description:
 * This program is a shell which executes Linux commands and allows
 * for redirection of input with ">", "<", and ">>" and allows 
 * piping with up to three commands. (The piping is incomplete though)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ARGS 20
#define MAX_PIPE_ARGS 3
#define BUFSIZE 1024

int input, output, append, usePipe;
char *inputFile;
char *outputFile;
FILE *fp1;
FILE *fp2;

int get_args(char* cmdline, char* args[]) 
{
  int i = 0;

  /* if no args */
  if((args[0] = strtok(cmdline, "\n\t ")) == NULL) 
    return 0; 

  while((args[++i] = strtok(NULL, "\n\t ")) != NULL) {
    if(i >= MAX_ARGS) {
      printf("Too many arguments!\n");
      exit(1);
    }
  }

  /* the last one is always NULL */
  return i;
}

//Checks for an input/output redirection like ">", "<", or ">>"
//It sets the position of the aforementioned symbols to NULL
//and input and output files to the argument after those symbols.
void checkIORedirect(char* args[])
{

        input = 0;
        output = 0;
        append = 0;

        int i;
        for(i = 0; args[i] != NULL; i++)
        {
                if(!strcmp(args[i], ">"))
                {
                        output = 1;
                        outputFile = args[i+1];
                        args[i] = NULL;
                }
                else if(!strcmp(args[i], "<"))
                {
                        input = 1;
			inputFile = args[i+1];
			args[i] = NULL;
                }
                else if(!strcmp(args[i], ">>"))
                {
                        append = 1;
                        outputFile = args[i+1];
                        args[i] = NULL;
                }
        }
}

//The pipe indexes are acquired by iterating through the arguments
//setting them to one of the pipeIndex int variables and then 
//setting the value of where the pipe is to NULL.
//
//Returns the amount of pipes
int getPipeIndexes(char* args[], int* pIndex1, int* pIndex2)
{
	*pIndex1 = -1;
	*pIndex2 = -1;	

	int i = 0;
	int pcount = 0;
	usePipe = 0;

	for(i = 0; args[i] != NULL; i++)
	{
		if(!strcmp(args[i], "|") && pcount == 0)
		{
			*pIndex1 = i;
			args[i] = NULL;
			pcount++;

			usePipe = 1;
		}
		else if(!strcmp(args[i], "|") && pcount == 1)
		{
			*pIndex2 = i;
			args[i] = NULL;
			pcount++;
		}
		else if(!strcmp(args[i], "|") && pcount > 1)
		{
			printf("Too many pipes, only accepting first two!\n");
			args[i] = NULL;
		}
	}
	
	return pcount;
}

//This method splits the pipe commands by setting a NULL value to each position
//where the pipes are located and then setting these arguments to separate
//arrays of strings: pcmd1, pcmd2, pcmd3 (if using two pipes).
//
//The indexes of the pipes are acquired from getPipeIndexes function above.
void splitPipeCommands(char* args[], int* pIndex1, int* pIndex2, int pcount, char* pcmd1[], char* pcmd2[], char* pcmd3[])
{
	if(pcount == 1)
	{
		int i;
		int cmdIndex = 0;
		for(i = 0; i < *pIndex1; i++)
		{
			pcmd1[cmdIndex] = args[i];
			cmdIndex++;
		}
		cmdIndex = 0;

		for(i = *pIndex1 + 1; args[i] != NULL; i++)
		{
			pcmd2[cmdIndex] = args[i];
			cmdIndex++;
		}
		//For some reason commands inputted to pcmd2 are broken as hell
		//I couldn't figure out the problem so this seems to work as a
		//"temporary" fix, unfortunately that means you can't input
		//more than one argument as a command after the first pipe.
		pcmd2[1] = NULL;
	}
	else if(pcount == 2)
	{
		int i;
		int cmdIndex = 0;
		for(i = 0; i < *pIndex1; i++)
		{
			pcmd1[cmdIndex] = args[i];
			cmdIndex++;
		}
		cmdIndex = 0;
		for(i = *pIndex1 + 1; i < *pIndex2; i++)
		{
			pcmd2[cmdIndex] = args[i];
			cmdIndex++;
		}
		//For some reason commands inputted to pcmd2 are broken as hell
		//I couldn't figure out the problem so this seems to work as a 
		//"temporary" fix, unfortunately that means you can't input
		//more than one argument as a command after the first pipe
		pcmd2[1] = NULL;

		cmdIndex = 0;
		for(i = *pIndex2 + 1; args[i] != NULL; i++)
		{
			pcmd3[cmdIndex] = args[i];
			cmdIndex++;
		}
	}
}

//This where all the execution happens, piping here is broken for two pipes.
//One pipe only commands seem to work OK if the command after the pipe is 
//only one argument long (there's an explanation as to why this is inside
//the above function).
void execute(char* cmdline) 
{
  int pid, pid2, async;
  char* args[MAX_ARGS];

  int nargs = get_args(cmdline, args);

  if(nargs <= 0) return;

  if(!strcmp(args[0], "quit") || !strcmp(args[0], "exit")) {
    exit(0);
  }

  /* check if async call */
  if(!strcmp(args[nargs-1], "&")) { async = 1; args[--nargs] = 0; }
  else async = 0;

  /*Pipe related*/
  int pIndex1, pIndex2;
  char* pcmd1[30];
  char* pcmd2[30];
  char* pcmd3[30];
  int pcount = getPipeIndexes(args, &pIndex1, &pIndex2);

  if(usePipe)
  {
	splitPipeCommands(args, &pIndex1, &pIndex2, pcount, pcmd1, pcmd2, pcmd3);
	if(pcount == 1)
	{
		checkIORedirect(pcmd1);
		checkIORedirect(pcmd2);
	}
	else if(pcount == 2)
	{
		checkIORedirect(pcmd1);
                checkIORedirect(pcmd2);
		checkIORedirect(pcmd3);
	}
  }
  else
  {
	checkIORedirect(args);
  }

  if(output)
  {
	if((fp1 = fopen(outputFile, "w")) == NULL)
	{
		printf("Output to file failed!\n");
	}
  }
  else if(append)
  {
  	if((fp1 = fopen(outputFile, "a")) == NULL)
	{
		printf("Append to file failed!\n");
	}
  } 
  
  if(input)
  {
  	if((fp2 = fopen(inputFile, "r")) == NULL)
	{
		printf("Input from file failed!\n");
	}
  }

  int fd[2];
  int fd2[2];

  if(usePipe) 
  {
	if(pipe(fd) == -1)
	{
		perror("pipe failed");
		exit(1);
	}
  }

  pid = fork();
  if(pid == 0) { /* child process */

    if(input)
    {
	dup2(fileno(fp2), STDIN_FILENO);
	fclose(fp2);
	input = 0;
    }
    if(output || append)
    {
	dup2(fileno(fp1), STDOUT_FILENO);
	fclose(fp1);
	output = 0;
	append = 0;
    }

    if(usePipe)
    {
	if(pcount == 2)
	{
		if(pipe(fd2) == -1)
		{
			perror("pipe failed");
			exit(1);
		}
		pid2 = fork();
		if(pid2 == -1)
		{
			perror("fork 2 failed");
			exit(1);
		}
		if(pid2 == 0) //Child 2
		{
			close(fd[1]);
			close(fd[0]);

			close(fd2[1]);
			dup2(fd2[0], 0);
			close(fd2[0]);

			execvp(pcmd2[0], pcmd2);
			perror("exec second child failed");
			exit(-1);
		}
		else //Child 1
		{
			close(fd[1]);
			dup2(fd[0], 0);
			close(fd[0]);
			close(fd2[0]);
			dup2(fd2[1], 1);
			close(fd2[1]);
			
			execvp(pcmd3[0], pcmd3);
			perror("exec first child failed");
			exit(-1);
		}
	}
	else if(pcount == 1)
	{

		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		printf("pcmd1=%s\n", pcmd1[0]);
		execvp(pcmd1[0], pcmd1);
		perror("exec child failed");
		exit(-1);
		
	}
	
    }
    else
    {
	execvp(args[0], args);
    	/* return only when exec fails */
   	perror("exec failed");
    	exit(-1);
    }

  } else if(pid > 0) { /* parent process */

    if(usePipe)
    {
	pid=fork();
	if(pid==0)
	{
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);
		close(fd[0]);

		printf("pcmd2 contents: \n");
		int j;
		for(j = 0; pcmd2[j] != NULL; j++)
		{
			printf("%s\n", pcmd2[j]);
		}
		//FIXME: Something's wrong with pcmd2
		execvp(pcmd2[0], pcmd2);
		perror("exec second command failed");
	}
	else
	{
		int status;
		close(fd[0]);
		close(fd[1]);
		waitpid(pid, &status, 0);
	}
    }

    if(!async)
    {
	waitpid(pid, NULL, 0);
    }
    else
    {
	 printf("this is an async call\n");
    }
  } else { /* error occurred */
    perror("fork failed");
    exit(1);
  }
}

int main (int argc, char* argv [])
{
  char cmdline[BUFSIZE];
  
  for(;;) {
    printf("COP4338$ ");
    if(fgets(cmdline, BUFSIZE, stdin) == NULL) {
      perror("fgets failed");
      exit(1);
    }
    execute(cmdline);
    usePipe = 0;
    fflush(stdout);
  }
  return 0;
}
