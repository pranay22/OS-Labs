#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Lab 1: Task 1.1:*/


/* TO_DO: Change to fprintf(); remove source from the answer of Question 1.1
Change Question 1.1 if needed. */


//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID, test;
	int childec;
	

	printf("This is parent. PID is: %d\n", getpid());
	printf("Inside Parent. PID is: %d\n", getpid());
	// Attempt to fork
	cPID = fork();		//fork() returns  0 to the child process and  PID of the child process to the parent process
	if(cPID > 0){
		//catch one return of fork to parent (i.e. PID of Child)
		printf("Child created. PID: %d\n", cPID);
	}
	usleep(150000);		//delay of 150ms after fork()  -- usleep(uSec)
	if(cPID >= 0){
	//fork sucessful
	
		if (cPID == 0){
			//inside child process
			usleep(500000);		//500ms delay in child process  -- usleep(uSec)
			my_value = 18951;	//changing 'my_value' inside child
			printf("I am child. PID : %d; my_value: %d\n", getpid(), my_value);
			
		}
		else {
			//in parent process
			printf("Inside parent again. PID is: %d; my_value: %d\n", getpid(), my_value);
			//Waiting for child
			test = wait(&childec);
			if (test == -1){
				printf("Failed in wait(). Error: %s \n", strerror(errno));
				exit(0);
			}
			printf("what wait() returns? %d\n", test);
			printf("Waited for child. Child now terminated. Exit Code: %d; my_value: %d\n", childec, my_value);
		}
	}
	else{
		printf("Failed to fork. Error : %s \n", strerror(errno));
		exit(0);
	}
	return 0;
}

/* Question 1.1: If program is executed 100 times PID of parent & child will be different. As at different execution times PID will be different (obviously). 
my_value: Parent prints: 42, Child prints: 18951.
Reason: Process cna change their own variables independently and they will change without any noticible effect on other process which are fork() of parent, siblings and descendents.
But OS share pages initially. So we can use mmap() to reallx share global vars.
Source: stackoverflow.com/questions/4298678/after-forking-are-global-variables-shared */
