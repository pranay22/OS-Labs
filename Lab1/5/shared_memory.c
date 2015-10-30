#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

#define SLEEP 150000
#define CSLEEP 500000

int my_value=42; // global variable

int main (int argc, char ** argv)
{
    pid_t cpid=0;
    pid_t mypid=0;

#define NAME "/DEEDS_lab1_shm"

    //Creating a SMO
    int fd= shm_open(NAME,O_RDWR|O_CREAT,S_IRWXU);
    if(fd==-1)
    {
        //Failure
        fprintf(stderr,"Failuere to create the SMO\n %s\n", (strerror(errno)));
        exit(EXIT_FAILURE);
    }
    
    //Re-sizing the SMO
    if(ftruncate(fd,256)==-1)
    {
        fprintf(stderr,"Failure to resize the SMO\n%s\n",(strerror(errno)));
        exit(EXIT_FAILURE);
            
    }
    //mapping the object
    void * ptr = mmap(NULL,256,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if((ptr==MAP_FAILED))
    {
        perror("Mapping Failed");
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
            my_value=18951;
            fprintf(stdout,"I'm the child, PID=%d, my_value=%d\n",mypid,
            my_value);
            usleep(CSLEEP); // child sleeping
            //Getting the parent msg,
            fprintf(stdout,"Message from parent: %s\n", (char*)ptr);
            //Sending a msg to the parent
            char msg[127];
            sprintf(msg,"Hi, I'm the child, PID=%d my_value=%d", mypid, my_value);
            strncpy(((char*)ptr)+128,msg,127);
            munmap(ptr,256);
            exit(EXIT_SUCCESS);

        }
        if(cpid>0)
        {
            //In the parent
            mypid=getpid();
            fprintf(stdout,"I'm the parent, PID=%d, my_value=%d\n",mypid, my_value);
            char msg[127];
            //Writing msg to the SMO
            sprintf(msg,"HI, I'm your parent, PID=%d, my_value=%d", mypid, my_value);
            strncpy((char*)ptr, msg, 127);
            //receiving the childs messge

            wait(NULL); // waiting for the child
            fprintf(stdout,"Message from the child: %s\n",((char*)ptr)+128);
            fprintf(stdout,"CHILD TERMINATED\n");
            //unmapping and closing
            munmap(ptr,256);
            shm_unlink(NAME);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
