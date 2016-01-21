/* Getopt-code "inspired" by: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

static volatile int keepRunning = 1;

void show_help(){
	printf("How to use this program:\n");
	printf("Consumer mode: Add parameters for mode, name and consumption rate.\n");
	printf("Example: prodCon -c -nExample -r1000\n\n");
	printf("Producer mode: Add parameters for mode, name, message and consumption rate.\n");
	printf("Example: prodCon -p -nExample -r1000 -mExampleMessage\n");
}

/*
* Parse a string to integer
*/
static int parse_to_int (const char * msg)
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

void intHandler(int dummy) {
	keepRunning = 0;
	printf("Program was cancelled\n");
}

/*
 * Required CLI-parameters: mode, name, rate, message
 */
int main(int argc, char* argv[]){
	
	signal(SIGINT,intHandler);
	
	int isProducer = 0;
	int isConsumer = 0;
	char* name = NULL;
	char* rate_arg = NULL;
	int rate = 0;
	char* message = NULL;
	int c;	
	while ((c = getopt(argc, argv, "hpcn:r:m:")) != -1){
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
			case '?':
				fprintf(stderr, "Unknown option: %c\n", optopt);
				return 1;
			default:
				abort();
		}
	}

	/* Evaluation of options*/
	if (isProducer && isConsumer){
		fprintf(stderr, "I can't be producer and consumer at the same time, moron!\n");
		return 1;
	}
	if (isProducer){
		if (name == NULL){
			fprintf(stderr, "Name missing. I can't work like this!\n");
			return 1;
		}
		if (message == NULL){
			fprintf(stderr, "Message missing. I can't work like this!\n");
			return 1;
		}
		if (rate_arg == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return 1;
		}
		rate = parse_to_int(rate_arg);
		// TODO: Insert rate conversion and actual creation
		unsigned int time_to_wait = 1000000/rate;
		while(keepRunning){
			printf("Item produced\n");
			usleep(time_to_wait);
		}
		return 0;
	}
	else if (isConsumer){
		if (name == NULL){
			fprintf(stderr, "Name missing. I can't work like this!\n");
			return 1;
		}
		if (rate_arg == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return 1;
		}
		rate = parse_to_int(rate_arg);		
		unsigned int time_to_wait = 1000000/rate;
		while(keepRunning){
			printf("Item consumed\n");
			usleep(time_to_wait);
		}
		return 0;
	}
	else{
		fprintf(stderr, "I must be either consumer or producer you twerp!\n");
		return 1;
	}

}
