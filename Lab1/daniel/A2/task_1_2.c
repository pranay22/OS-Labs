/* Copyright */

// usleep
#include <unistd.h>

// pthread
#include <pthread.h>

// fprintf
#include <stdio.h>

// exit
#include <stdlib.h>

// errno
#include <errno.h>

// string
#include <string.h>


#define SLEEP 150000
#define CSLEEP 500000
#define error_checker(stat, msg)\
  if(stat) {\
   fprintf(stderr, "Error:\nDescription:\n%s.\nNumber:\n%s\n\n", msg, strerror(errno));\
   exit(EXIT_FAILURE);\
  }


int my_value = 42;


void* child_thread_function(void* arg) {
  // Delay 150ms
  int usleep_status = usleep(SLEEP);
  error_checker(usleep_status == -1, "Child:\tUSleep failed");

  // Change value of global var
  my_value = 18951;
  fprintf(stderr, "Child:\t%d\t%d\n", getpid(), my_value);

  // Delay 500ms
  usleep_status = usleep(CSLEEP);
  error_checker(usleep_status == -1, "Child:\tUSleep failed");

  pthread_exit(0);
}

int main(int argc, char** argv) {
  // Create child thread
  // NOTE: thread creation; returns 0 on success, error number on error
  pthread_t child_thread;
  int thread_status = pthread_create(&child_thread, NULL, child_thread_function, NULL);
  error_checker(thread_status != 0, "PThread create failed");
  
  // Delay 150ms
  int usleep_status = usleep(SLEEP);
  error_checker(usleep_status == -1, "Parent:\tUSleep failed");
    
  // Parent
  fprintf(stderr, "Parent:\t%d\t%d\n", getpid(), my_value);
  fprintf(stderr, "Parent:\tchild created:\t\t%d\n", getpid());

  // Wait for child to return
  int* ptr_to_thread_ret;
  thread_status = pthread_join(child_thread, (void**)&ptr_to_thread_ret);
  error_checker(thread_status != 0, "PThread join failed");

  fprintf(stderr, "Parent:\tchild terminated:\t%d\t%d\n", getpid(), my_value);

  return 0;
}
