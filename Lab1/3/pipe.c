#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define error_check(stat, msg)\
	if(!(stat))\
	 fprintf(stderr, "Error Description: %s. Error Number : %s \n", msg, strerror(errno));\

/* Lab 1: Task 3.1:*/


/* TO_DO:  --
Change Question 3.1 if needed. */

//buffer size
#define MAX_PIPE_BUFFER 1024

//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID, test;
	int cProc;
	int fd[2];
	char pipeBuffer[MAX_PIPE_BUFFER];
	int pipeForkResult;

	fprintf(stderr, "This is parent. PID is: %d\n", getpid());
	fprintf(stderr, "Inside Parent. PID is: %d\n", getpid());
	//Create Pipe
	pipeForkResult = pipe(fd);
	if(pipeForkResult == -1){
		fprintf(stderr, "Failed to create pipe. Error : %s \n", strerror(errno));
		exit(2);
	}
		

	// Attempt to fork
	cPID = fork();		//fork() returns  0 to the child process and  PID of the child process to the parent process
	if(cPID > 0){
		//catch one return of fork to parent (i.e. PID of Child)
		fprintf(stderr, "Child created. PID: %d\n", cPID);
	}
	usleep(150000);		//delay of 150ms after fork()  -- usleep(uSec)
	if(cPID >= 0){
	//fork sucessful
	
		if (cPID == 0){
			//inside child process
			usleep(500000);		//500ms delay in child process  -- usleep(uSec)
			my_value = 18951;	//changing 'my_value' inside child
			fprintf(stderr, "I am child. PID : %d; my_value: %d\n", getpid(), my_value);
			
			//Close write pipe
			int statCloseWriteChild = close(fd[1]);
			if(statCloseWriteChild == 0)
				fprintf(stderr, "Closing write end of pipe");
			else{
				fprintf(stderr, "Failed during close(fd[1]) in child. Error: %s \n", strerror(errno));
				exit(2);
			}
			//error_check(0 == close(fd[1]), "Closing write end of pipe");
			//Read from pipe
			int statReadFromParent = read(fd[0], pipeBuffer, MAX_PIPE_BUFFER);
			if(statReadFromParent >= 0)
				fprintf(stderr, "Reading from pipe");
			else{
				fprintf(stderr, "Failed during read() in child. Error: %s \n", strerror(errno));
				exit(2);
			}
			//error_check(0 <= read(fd[0], pipeBuffer, MAX_PIPE_BUFFER), "Reading from pipe");
			//Print the data read from pipe
			fprintf(stderr, "Data from pipe after reading: %s \n", pipeBuffer);
			// Close read pipe
			int statCloseReadChild = close(fd[0]);
			if(statCloseReadChild == 0)
				fprintf(stderr,"Closing read end of pipe\n");
			else{
				fprintf(stderr, "Failed during close(fd[0]) in child. Error: %s \n", strerror(errno));
				exit(2);
			}
			//err_check(0 == close(fd[0]), "Closing read end of pipe");

			
		}
		else {
			//in parent process
			fprintf(stderr, "Inside parent again. PID is: %d; my_value: %d\n", getpid(), my_value);
			
			sprintf(pipeBuffer, "Hi, I am your parent. My PID: %d and my_value: %d", getpid(), my_value);
			printf("Data to send to pipe:%s\n", pipeBuffer);

			//Close read pipe
			int statCloseReadParent = close(fd[0]);
			if(statCloseReadParent == 0)
				fprintf(stderr,"Closing read end of pipe\n");
			else{
				fprintf(stderr, "Failed during close(fd[0]) in parent. Error: %s \n", strerror(errno));
				exit(2);
			}
			//error_check(0 == close(fd[0]), "Closing read end of pipe");
			//Write to pipe
			int statWriteParent = write(fd[1], pipeBuffer, sizeof(pipeBuffer));
			if(statWriteParent >= 0)
				fprintf(stderr, "Writing to pipe");
			else{
				fprintf(stderr, "Failed during write() in parent. Error: %s \n", strerror(errno));
				exit(2);
			}
			//error_check(0 <= write(fd[1], pipeBuffer, sizeof(pipeBuffer)), "Writing to pipe");
			//Close write pipe
			int statCloseWriteParent = close(fd[1]);
			if(statCloseWriteParent == 0)
				fprintf(stderr, "Closing write end of pipe");
			else{
				fprintf(stderr, "Failed during close(fd[1]) in parent. Error: %s \n", strerror(errno));
				exit(2);
			}
			//error_check(0 == close(fd[1]), "Closing write end of pipe");

			//Waiting for child
			test = wait(&cProc);
			if (test == -1){
				fprintf(stderr, "Failed in wait(). Error: %s \n", strerror(errno));
				exit(1);
			}
			fprintf(stderr, "Waited for child. Child now terminated. PID: %d; Exit Code: %d; my_value: %d\n",cPID, cProc, my_value);
		}
	}
	else{
		fprintf(stderr, "Failed to fork. Error : %s \n", strerror(errno));
		exit(2);
	}
	return 0;
}

/* Question 2.1:  */
