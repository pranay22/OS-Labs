#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

#define SLEEP 150000
#define CSLEEP 500000

int my_value=42; // global variable

int main (int argc, char ** argv)
{
    pid_t cpid=0;
    pid_t mypid=0;
    int pipefd[2];

    //Creating a pipe
    if(pipe(pipefd)){
        fprintf(stderr,"%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    cpid=fork();
    if(cpid<0){
        perror("Fork unsuccessful\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        usleep(SLEEP);//both processes sleeps
        if(cpid==0)
        {
            //In the child
            mypid=getpid();
            close(*(pipefd+1)); // closing the write end of the pipe
            my_value=18951;
            fprintf(stdout,"I'm the child, PID=%d, my_value=%d\n",mypid,
            my_value);
            usleep(CSLEEP); // child sleeping
            char message[256];
            if(read(*(pipefd),message,255)==-1){//reading from the pipe
                perror("Error while reading from the pipe\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stdout,"%s\n",message);
            }
            exit(EXIT_SUCCESS);

        }
        if(cpid>0)
        {
            //In the parent
            mypid=getpid();
            close(*pipefd); // closing the read end of the pipe
            fprintf(stdout,"I'm the parent, PID=%d, my_value=%d\n",mypid, my_value);
            char message[256];
            sprintf(message,"Hi, I am your parent. My PID=%d and my_value=%d",mypid,my_value);
            int count=strlen(message);
            write(*(pipefd+1),message,count+1); // writing to the pipe
            wait(NULL); // waiting for the child;
            fprintf(stdout,"CHILD TERMINATED\n");
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
