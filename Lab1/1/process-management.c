#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CSLEEP 500000
#define SLEEP 150000

/* Lab 1: Task 1.1:*/


/* TO_DO:  remove source from the answer of Question 1.1
Change Question 1.1 if needed. */


//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID ;
	
	fprintf(stdout, "This is parent. PID is: %d\n", getpid());
	//fprintf(stdout, "Inside Parent. PID is: %d\n", getpid());
    
    if((cPID=fork())<0){
        fprintf(stderr,"Fork Error");
        exit(1);
    }
    else
    {
        usleep(SLEEP);
        if(cPID==0){
        my_value=18951;
        fprintf(stdout,"I'm the child, PID=%d, my_value=%d\n", getpid(), my_value);
        usleep(CSLEEP);
        exit(0);
        }
        if(cPID>0)
        {
            fprintf(stdout,"I'm the parent, PID=%d,my_value=%d\n",getpid(),my_value);
            fprintf(stdout,"Child fork successful\n");
            wait(NULL);
            fprintf(stdout,"CHILD Terminated\n");
            exit(0);
        }

    }
	return 0;
}

/* Question 1.1: If program is executed 100 times PID of parent & child will be different. As at different execution times PID will be different (obviously). 
my_value: Parent prints: 42, Child prints: 18951.
Reason: Process cna change their own variables independently and they will change without any noticible effect on other process which are fork() of parent, siblings and descendents.
But OS share pages initially. So we can use mmap() to reallx share global vars.
Source: stackoverflow.com/questions/4298678/after-forking-are-global-variables-shared */
