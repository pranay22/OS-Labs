/* Getopt-code "inspired" by: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt*/
#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include "fifo.h"

void show_help(){
	printf("How to use this program:\n");
	printf("Consumer mode: Add parameters for mode, name, consumption rate and file to access.\n");
	printf("Example: producer-consumer -c -nExample -r1000 -l~/example\n\n");
	printf("Producer mode: Add parameters for mode, name, message, consumption rate and file to access.\n");
	printf("Example: producer-consumer -p -nExample -r1000 -mExampleMessage -l~/example\n");
}

/*
 * Parse a string to integer
 */
static unsigned int parse_to_int (const char * msg)
{
	unsigned int val = 0;
	unsigned int ten=1; 
	int i;
	unsigned int length;
	if(msg!=NULL)
	{
		length = strlen(msg);
		for(i=length-1; i>-1; i--)
		{
			if(msg[i]<='9' && msg[i]>='0')
			{
				val += ((int)msg[i]-'0')*ten;
				ten*=10;
			}
			else
			{
				val = -1;
				break;
			}
		}
		if(val>=0)
			return val;
		else
			return 0;
	}
	return 0;
}

/*
 * Parse a string to integer
 */
static unsigned long long parse_to_llu (const char * msg)
{
	unsigned long long val = 0;
	unsigned int ten=1; 
	int i;
	unsigned int length;
	if(msg!=NULL)
	{
		length = strlen(msg);
		for(i=length-1; i>-1; i--)
		{
			if(msg[i]<='9' && msg[i]>='0')
			{
				val += ((int)msg[i]-'0')*ten;
				ten*=10;
			}
			else
			{
				val = -1;
				break;
			}
		}
		if(val>=0)
			return val;
		else
			return 0;
	}
	return 0;
}

void intHandler(int dummy) {
	printf("Program was cancelled\n");
	exit(EXIT_SUCCESS);
}

/**
 * Encapsules the code that writes a data item to a file
 */
void write_to_file(FILE* file, struct data_item* item){
	if (file != NULL) fprintf(file, "%d,%llu,%s\n", item->qid, item->time, item->msg);
	else{
		fprintf (stderr, "File could not be opened!\n");
		exit( EXIT_FAILURE);
	}
}

struct data_item* read_from_file(FILE* file){
	if (file == NULL){
		fprintf(stderr, "File could not be opened!\n");
		exit (EXIT_FAILURE);
	}
	else{
		unsigned int qid = 0;
		unsigned long long time = 0;
		const char* separator = ",";
		char* text = malloc(128);
		char* msg;
		
		fscanf(file, "%s", text);
		struct data_item* item = malloc(sizeof(struct data_item));
		char* token = strtok(text,separator);
		int tokenCounter = 0;
		while(token != NULL){
			if (tokenCounter == 0) qid = parse_to_int(token);
			else if (tokenCounter == 1) time = parse_to_llu(token);
			else if (tokenCounter == 2) asprintf(&msg, "%s", token);
			token = strtok(NULL, separator);
			tokenCounter++;
		}
		item->qid = qid;
		item->time = time;
		item->msg = msg;
		free(text);
		return item;
	}
}

/*
 * Required CLI-parameters: mode, name, rate, message, path
 */
int main(int argc, char* argv[]){
	
	signal(SIGINT,intHandler);
	
	int isProducer = 0;
	int isConsumer = 0;
	char* name = NULL;
	char* rate_arg = NULL;
	int rate = 0;
	char* message = NULL;
	char* path = "/dev/deeds_fifo";
	int c;	
	while ((c = getopt(argc, argv, "hpcn:r:m:l:")) != -1){
		switch(c){
			case 'h':
				show_help();
				return 1;
			case 'p':
				isProducer = 1;
				break;
			case 'c':
				isConsumer = 1;
				break;
			case 'n':
				name = optarg;
				break;
			case 'r':
				rate_arg = optarg;
				break;
			case 'm':
				message = optarg;
				break;
			case 'l':
				path = optarg;
				break;
			case '?':
				fprintf(stderr, "Unknown option: %c\n", optopt);
				return EXIT_FAILURE;
			default:
				abort();
		}
	}

	/* Evaluation of options*/
	if (isProducer && isConsumer){
		fprintf(stderr, "I can't be producer and consumer at the same time!\n");
		return EXIT_FAILURE;
	}
	if (isProducer){	//We've got a producer
		if (name == NULL){
			fprintf(stderr, "Name missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		if (message == NULL){
			fprintf(stderr, "Message missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		if (rate_arg == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		if (path == NULL){
			fprintf(stderr, "Path missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		rate = parse_to_int(rate_arg);
		unsigned int time_to_wait = 1000000/rate;
		//In this loop the actual production of items is done.
		while(1){
			FILE *file = fopen(path,"a");
			struct data_item item = {.msg=message, .time=time(0)};
			write_to_file(file, &item);
			fclose(file);
			usleep(time_to_wait);
		}
		return 0;
	}
	else if (isConsumer){	//We've got a consumer
		if (name == NULL){
			fprintf(stderr, "Name missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		if (rate_arg == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		if (path == NULL){
			fprintf(stderr, "Path missing. I can't work like this!\n");
			return EXIT_FAILURE;
		}
		rate = parse_to_int(rate_arg);		
		unsigned int time_to_wait = 1000000/rate;
		//In this loop the actual consumption of items is done.
		while(1){
			printf("Item consumed\n");
			FILE* file = fopen(path, "r");
			struct data_item* item = read_from_file(file);
			printf("Read item no. %d, produced at %llu, with message '%s'\n", item->qid, item->time, item->msg);	
			free(item->msg);		
			free(item);
			usleep(time_to_wait);
		}
		return 0;
	}
	else{
		fprintf(stderr, "I must be either consumer or producer!\n");
		return EXIT_FAILURE;
	}

}
