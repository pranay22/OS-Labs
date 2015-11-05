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

// MQ
#include <fcntl.h> // O_* constants
#include <sys/stat.h> // mode constants (unused)
#include <mqueue.h>


#define SLEEP 150000
#define CSLEEP 500000
#define BUFFER_SIZE 80
#define NAME "/DEEDS_lab1_mq"
#define error_checker(stat, msg)\
  if(stat) {\
   fprintf(stderr, "Error:\nDescription:\n%s.\nNumber:\n%s\n\n", msg, strerror(errno));\
   mq_unlink(NAME);\
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
    error_checker(usleep_status == -1, "Child: USleep failed");

    // POSIX message queue
    int mq_status;
    mqd_t mq = mq_open(NAME, O_RDONLY);
    error_checker(mq == -1, "Child: MQ open failed");

    char buffer[BUFFER_SIZE];
    mq_status = mq_receive(mq, buffer, BUFFER_SIZE, NULL);
    error_checker(mq_status == -1, "Child: MQ receive failed");

    fprintf(stderr, "Child:\tMessage: %s\n", buffer);

    mq_status = mq_close(mq);
    error_checker(mq_status == -1, "Child: MQ close failed");

  // Parent
  } else {
    fprintf(stderr, "Parent:\t%d\t%d\n", getpid(), my_value);
    fprintf(stderr, "Parent:\tchild created:\t\t%d\n", pid);

    // POSIX message queue
    int mq_status;
    struct mq_attr attr = {0, 1, BUFFER_SIZE, 0};
    //attr.mq_flags = 0; attr.mq_maxmsg = 1; attr.mq_msgsize = BUFFER_SIZE; attr.mq_curmsgs = 0;
    mqd_t mq = mq_open(NAME, O_CREAT | O_WRONLY, 0644, &attr);
    error_checker(mq == -1, "Parent: MQ open failed");

    int size;
    char message[BUFFER_SIZE];
    size = sprintf(message, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

    mq_status = mq_send(mq, message, size, 1);
    error_checker(mq_status == -1, "Parent: MQ send failed");

    // Wait for child to return
    // NOTE: returns process ID if OK, -1 on error
    int wait_status;
    pid_t id = wait(&wait_status);
    error_checker(wait_status == -1, "Parent: Wait failed");

    mq_status = mq_close(mq);
    error_checker(mq_status == -1, "Parent: MQ close failed");

    mq_status = mq_unlink(NAME);
    error_checker(mq_status == -1, "Parent: MQ unlink failed");

    fprintf(stderr, "Parent:\tchild terminated:\t%d\t%d\n", pid, my_value);
  }

  return 0;
}
