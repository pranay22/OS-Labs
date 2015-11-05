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


//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID ;
	
	// fprintf(stderr, "This is parent. PID is: %d\n", getpid());
    
    if ((cPID=fork())<0) {
        fprintf(stderr, "Fork Error\n");
        exit(1);
    }
    else
    {
        if (usleep(SLEEP) < 0) {
            fprintf(stderr, "USleep Error\n");
            exit(1);
        }
        if (cPID==0) {
            my_value=18951;
            fprintf(stderr, "I'm the child, PID=%d, my_value=%d\n", getpid(), my_value);
            if (usleep(CSLEEP) < 0) {
                fprintf(stderr, "USleep Error\n");
                exit(1);
            }
            exit(0);
        }
        else // if(cPID>0)
        {
            fprintf(stderr, "I'm the parent, PID=%d,my_value=%d\n", getpid(), my_value);
            fprintf(stderr, "Child fork successful: PID=%d\n", cPID);
            int wait_status;
            wait(&wait_status);
            if (wait_status < 0) {
                fprintf(stderr, "Wait Error\n");
                exit(1);
            }
            fprintf(stderr, "Child Terminated\n");
            exit(0);
        }

    }
	return 0;
}
