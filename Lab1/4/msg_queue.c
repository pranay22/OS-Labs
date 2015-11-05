#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define error_checker(stat, msg)\
	if(!(stat))\
	 fprintf(stderr, "Error! Description: %s. Number : %s \n", msg, strerror(errno));\


#define SLEEP 150000
#define CSLEEP 500000
//buffer size
#define MAX_MQ_BUFFER 1024

int my_value=42; // global variable

int main (int argc, char ** argv)
{
    pid_t cpid=0;
    pid_t mypid=0;
    mqd_t msgQ;
    struct mq_attr MQAttr;
    char *buffer;
    char MQBuffer[MAX_MQ_BUFFER];
    
    //Setting initial attributes
    MQAttr.mq_flags = 0; 	// Blocking queue  
    MQAttr.mq_maxmsg = 1;  
    MQAttr.mq_msgsize = MAX_MQ_BUFFER;  
    MQAttr.mq_curmsgs = 0;

#define NAME "/DEEDS_lab1_mq"

    //Creating a MQ
    msgQ = mq_open(NAME, O_CREAT | O_WRONLY, 0644, &MQAttr);
	if(msgQ==-1)
		fprintf(stderr, "Failed to create Message Queue. Error : %s \n", strerror(errno));

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

            //Child: Opens, closes MQ
            // Open queue with READ access
            msgQ = mq_open(NAME, O_RDONLY);
            error_checker(0 <= msgQ, "Opening MQ: \n");
            // Get attributes
            error_checker(0 <= mq_getattr(msgQ, &MQAttr), "reading attributes");	
            //Allocate buffer
            buffer = (char *) malloc(MQAttr.mq_msgsize + 1);
            // Read from queue
            error_checker(0 <= mq_receive(msgQ, buffer, MQAttr.mq_msgsize, NULL), "Reading from MQ:\n");	
            // Print the read data and free memory
            fprintf(stderr, "Data received : %s \n", buffer);
            free(buffer);
            //closing MQ in Child
            error_checker(0 <= mq_close(msgQ), "Child: Closing MQ");
            exit(EXIT_SUCCESS);
        }
        if(cpid>0)
        {
            //In the parent
            mypid=getpid();
            fprintf(stdout,"I'm the parent, PID=%d, my_value=%d\n",mypid, my_value);

            sprintf(MQBuffer, "Hi, I am your parent. My PID: %d and my_value: %d", getpid(), my_value);
            printf("Data to send to pipe:%s\n", MQBuffer);

            //parent: create, delete MQ (creation, deletion already done)
            // Open queue with WRITE access
            msgQ = mq_open(NAME, O_WRONLY);
            //Writing to MQ
            error_checker(0 <= mq_send(msgQ, MQBuffer, sizeof(MQBuffer), 1), "Writing to MQ\n");
            wait(NULL); // waiting for the child
            fprintf(stdout,"CHILD TERMINATED\n");
            //closing & deleting MQ
            error_checker(0 <= mq_close(msgQ), "Parent: Closing MQ");
            error_checker(0 <= mq_unlink(NAME), "Parent: Unlinking MQ");	
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
