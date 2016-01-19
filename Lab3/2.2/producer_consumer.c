/* Getopt-code "inspired" by: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void show_help(){
	printf("How to use this program:\n");
	printf("Consumer mode: Add parameters for mode, name and consumption rate.\n");
	printf("Example: prodCon -c -nExample -r1000\n\n");
	printf("Producer mode: Add parameters for mode, name, message and consumption rate.\n");
	printf("Example: prodCon -p -nExample -r1000 -mExampleMessage\n");
}

/*
 * Required CLI-parameters: mode, name, rate, message
 */
int main(int argc, char* argv[]){
	int isProducer = 0;
	int isConsumer = 0;
	char* name = NULL;
	char* rate = NULL;
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
				rate = optarg;
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
		if (rate == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return 1;
		}
		// TODO: Insert rate conversion and actual creation
		printf("Producer %s with a production rate of %s and message %s created.\n", name, rate, message);
		return 0;
	}
	else if (isConsumer){
		if (name == NULL){
			fprintf(stderr, "Name missing. I can't work like this!\n");
			return 1;
		}
		if (rate == NULL){
			fprintf(stderr, "Rate missing. I can't work like this!\n");
			return 1;
		}		
		//TODO: Insert rate conversion and actual creation
		printf("Consumer %s with a consumption rate of %s created.\n", name, rate);
		return 0;
	}
	else{
		fprintf(stderr, "I must be either consumer or producer you twerp!\n");
		return 1;
	}

}
