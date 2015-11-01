#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Lab 1: Task 1.2:*/

//Mentioned global variable
int my_value = 42;

/*
 * The child process' code put into a function for threading
 */
void* thread_func(void* in){
	if (usleep(150000) < 0) {  //delay of 150ms after pthread_create()  -- usleep(uSec)
    fprintf(stderr, "Child:\tUSleep Error\n");
    exit(1);
  }
	my_value = 18951;	//changing 'my_value' inside child
	fprintf(stderr, "I am child-thread. PID : %d; my_value: %d\n", getpid(), my_value);
	if (usleep(500000) < 0) {  //500ms delay in child process  -- usleep(uSec)
    fprintf(stderr, "Child:\tUSleep Error\n");
    exit(1);
  }
	int* retval = malloc(sizeof(int));
	*retval = 0;
	pthread_exit((void*)retval);
}

int main(){
	int  thread_err;
	int* thread_retval;	
	pthread_t my_thread;

	fprintf(stderr, "This is parent-thread. PID is: %d\n", getpid());
	// Attempt to create a new child-thread
	thread_err = pthread_create(&my_thread, NULL, thread_func, NULL);
	// Error handling for thread creation		
	if (thread_err){
		fprintf(stderr, "Child-thread creation failed with exit code %d\n", thread_err);
		exit(thread_err);
	}
	else{
		fprintf(stderr, "Child-thread created.\n");
	}	
	usleep(150000);		//delay of 150ms after pthread_create()  -- usleep(uSec)
	//in parent process
	printf("Inside parent-thread. PID is: %d; my_value: %d\n", getpid(), my_value);
	//Waiting for child
	thread_err = pthread_join(my_thread, (void**) &thread_retval);
	//Error handling for joining
	if (thread_err){
		fprintf(stderr, "Child-thread joining failed with exit code %d\n", thread_err);
		exit(thread_err);
	}
	else{
		fprintf(stderr, "Waited for child-thread. Child now terminated. Exit Code: %d; my_value: %d\n", *thread_retval, my_value);
	}
	return 0;
}
