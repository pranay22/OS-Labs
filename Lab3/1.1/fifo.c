#include <linux/module.h>
#include <linux/errno.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include <linux/semaphore.h>	/* for semaphores & mutex */

#define BUFFER_SIZE 32 /* Default FIFO buffer - unless changed */

MODULE_AUTHOR("DEEDS");
MODULE_DESCRIPTION("Lab 3 - 1.1 Solution");
MODULE_LICENSE("GPL");

//FIFO data item
struct data_item {
	unsigned int qid;	//queue sequence numberfor item
	unsigned long long time;//timestamp (sec) for item creation
	char *msg;		//NULL terminated C string
}



module_init(fifo_init);
module_exit(fifo_cleanup);
