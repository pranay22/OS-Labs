/* Copyright */

// fork + usleep
#include <unistd.h>

// wait
#include <sys/types.h>
#include <sys/wait.h>

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

int main(int argc, char** argv) {
  // parent / child identifier
  pid_t pid;

  // Create child process
  // NOTE: returns 0 in child, process ID of child in parent, -1 on error
  pid = fork();
  error_checker(pid == -1, "Fork failed");

  // Delay 150ms
  int usleep_status = usleep(SLEEP);
  error_checker(usleep_status == -1, "USleep failed");

  // Child
  if (pid == 0) {
    my_value = 18951;
    fprintf(stderr, "Child:\t%d\t%d\n", getpid(), my_value);

    // Delay 500ms
    int usleep_status = usleep(CSLEEP);
    error_checker(usleep_status == -1, "USleep failed");

  // Parent
  } else {
    fprintf(stderr, "Parent:\t%d\t%d\n", getpid(), my_value);
    fprintf(stderr, "Parent:\tchild created:\t\t%d\n", pid);

    // Wait for child to return
    // NOTE: returns process ID if OK, -1 on error
    int wait_status;
    pid_t id = wait(&wait_status);
    error_checker(wait_status == -1, "Wait failed");

    fprintf(stderr, "Parent:\tchild terminated:\t%d\t%d\n", pid, my_value);
  }

  return 0;
}
