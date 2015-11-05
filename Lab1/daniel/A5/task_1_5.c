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

// SMO
#include <sys/mman.h>
#include <sys/stat.h> // mode constants
#include <fcntl.h>    // O_* constants


#define SLEEP 150000
#define CSLEEP 500000
#define BUFFER_SIZE 80
#define NAME "/DEEDS_lab1_shm"
#define error_checker(stat, msg)\
  if(stat) {\
   fprintf(stderr, "Error:\nDescription:\n%s.\nNumber:\n%s\n\n", msg, strerror(errno));\
   shm_unlink(NAME);\
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

    // POSIX shared memory
    int shm_status;
    int shm = shm_open(NAME, O_RDWR, 0644);
    error_checker(shm == -1, "Child: SHM open failed");

    void* mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    error_checker(mem == MAP_FAILED, "Child: SHM map failed");

    fprintf(stderr, "Child:\tMessage: %s\n", (char*) mem);

    int size;
    char message[BUFFER_SIZE];
    size = sprintf(message, "Hi, I am your child. My PID=%d and my_value=%d", getpid(), my_value);

    strncpy((char*)mem, message, BUFFER_SIZE);

    shm_status = munmap(mem, BUFFER_SIZE);
    error_checker(shm_status == -1, "Child: SHM unmap failed");

  // Parent
  } else {
    fprintf(stderr, "Parent:\t%d\t%d\n", getpid(), my_value);
    fprintf(stderr, "Parent:\tchild created:\t\t%d\n", pid);

    // POSIX shared memory
    int shm_status;
    int shm = shm_open(NAME, O_CREAT | O_RDWR, 0644);
    error_checker(shm == -1, "Parent: SHM open failed");

    shm_status = ftruncate(shm, BUFFER_SIZE);
    error_checker(shm_status == -1, "Parent: SHM resize failed");

    void* mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    error_checker(mem == MAP_FAILED, "Parent: SHM map failed");

    int size;
    char message[BUFFER_SIZE];
    size = sprintf(message, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);

    strncpy((char*)mem, message, BUFFER_SIZE);

    // TODO: delay? ask shm for change? while (!changed) ...? -> fstat?
    //int usleep_status = usleep(999999);
    fprintf(stderr, "Parent:\tMessage: %s\n", (char*) mem);

    // Wait for child to return
    // NOTE: returns process ID if OK, -1 on error
    int wait_status;
    pid_t id = wait(&wait_status);
    error_checker(wait_status == -1, "Parent: Wait failed");

    shm_status = munmap(mem, BUFFER_SIZE);
    error_checker(shm_status == -1, "Parent: SHM unmap failed");

    shm_status = shm_unlink(NAME);
    error_checker(shm_status == -1, "Parent: SHM unlink failed");

    fprintf(stderr, "Parent:\tchild terminated:\t%d\t%d\n", pid, my_value);
  }

  return 0;
}
