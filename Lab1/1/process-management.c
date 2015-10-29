#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Lab 1: Task 1.1:*/


//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID, test;
	int childec;
	

	fprintf(stderr, "This is parent. PID is: %d\n", getpid());
	fprintf(stderr, "Inside Parent. PID is: %d\n", getpid());
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
			
			my_value = 18951;	//changing 'my_value' inside child
			fprintf(stderr, "I am child. PID : %d; my_value: %d\n", getpid(), my_value);
			usleep(500000);		//500ms delay in child process  -- usleep(uSec)
		}
		else {
			//in parent process
			printf("Inside parent again. PID is: %d; my_value: %d\n", getpid(), my_value);
			//Waiting for child
			test = wait(&childec);
			if (test == -1){
				fprintf(stderr, "Failed in wait(). Error: %s \n", strerror(errno));
				exit(1);
			}
			fprintf(stderr, "Waited for child. Child now terminated. PID: %d; Exit Code: %d; my_value: %d\n",cPID, childec, my_value);
		}
	}
	else{
		fprintf(stderr, "Failed to fork. Error : %s \n", strerror(errno));
		exit(2);
	}
	return 0;
}
