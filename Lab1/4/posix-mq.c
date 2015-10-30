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
	 fprintf(stderr, "Error Description: %s. Error Number : %s \n", msg, strerror(errno));\

/* Lab 1: Task 4.1:*/


/* TO_DO:  --
Answer to Question 3.1 */

//buffer size
#define MAX_MQ_BUFFER 1024
//MessageMQ name
#define MQNAME "/DEEDS_lab1_mq"

//Mentioned global variable
int my_value = 42;

int main(){
	pid_t cPID, test;
	int cProc;
	mqd_t msgQ;
	struct mq_attr MQAttr;
	char *buffer;
	
	char MQBuffer[MAX_MQ_BUFFER];
	//int pipeForkResult;


	//Setting initial attributes
	MQAttr.mq_flags = 0; 	// Blocking queue  
	MQAttr.mq_maxmsg = 10;  
	MQAttr.mq_msgsize = MAX_MQ_BUFFER;  
	MQAttr.mq_curmsgs = 0;  

	fprintf(stderr, "This is parent. PID is: %d\n", getpid());
	fprintf(stderr, "Inside Parent. PID is: %d\n", getpid());

	//MQ creation
	msgQ = mq_open(MQNAME, O_CREAT, 0644, &MQAttr);
	if(msgQ <0)
		fprintf(stderr, "Failed to create Message Queue. Error : %s \n", strerror(errno));
	else
		fprintf(stderr, "Sucessfully created Message Queue.");
	//err_check(0 <= mqd, "creating queue");

		

	// Attempt to fork
	cPID = fork();		//fork() returns  0 to the child process and  PID of the child process to the parent process
	if(cPID > 0){
		//catch one return of fork to parent (i.e. PID of Child)
		fprintf(stderr, "Child created. PID: %d\n", cPID);
	}
	usleep(150000);		//delay of 150ms after fork()  -- usleep(uSec)
	if(cPID >= 0){
	//fork sucessful
	
		if (cPID == 0){
			//inside child process
			usleep(500000);		//500ms delay in child process  -- usleep(uSec)
			my_value = 18951;	//changing 'my_value' inside child
			fprintf(stderr, "I am child. PID : %d; my_value: %d\n", getpid(), my_value);
			
			//Child: Opens, closes MQ
			// Open queue with READ access
			msgQ = mq_open(MQNAME, O_RDONLY);
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

			
		}
		else {
			//in parent process
			fprintf(stderr, "Inside parent again. PID is: %d; my_value: %d\n", getpid(), my_value);
			
			sprintf(MQBuffer, "Hi, I am your parent. My PID: %d and my_value: %d", getpid(), my_value);
			printf("Data to send to pipe:%s\n", MQBuffer);
			

			//parent: create, delete MQ (creation, deletion already done)
			// Open queue with WRITE access
			msgQ = mq_open(MQNAME, O_WRONLY);
			error_checker(0 <= msgQ, "Opening MQ\n");
			//Writing to MQ
			error_checker(0 <= mq_send(msgQ, MQBuffer, sizeof(MQBuffer), 1), "Writing to MQ\n");
			
			//Closing the MQ
			error_checker(0 <= mq_close(msgQ), "Closing MQ\n");


			//Waiting for child
			test = wait(&cProc);
			if (test == -1){
				fprintf(stderr, "Failed in wait(). Error: %s \n", strerror(errno));
				exit(1);
			}
			fprintf(stderr, "Waited for child. Child now terminated. PID: %d; Exit Code: %d; my_value: %d\n",cPID, cProc, my_value);
			
			//closing & deleting MQ
			// Close the queue
			error_checker(0 <= mq_close(msgQ), "Closing MQ");
			// Unlink queue
			error_checker(0 <= mq_unlink(MQNAME), "Unlinking MQ");	

		}
	}
	else{
		fprintf(stderr, "Failed to fork. Error : %s \n", strerror(errno));
		exit(2);
	}
	return 0;
}

/* Question 2.1:  */
