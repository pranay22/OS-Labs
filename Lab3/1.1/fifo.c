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
//Initialization of FIFO buffer
if(!arg[0]) {
	struct data_item buffer[BUFFER_SIZE];
}
else {
	struct data_item buffer[arg[0]];
}
//For race condition handling
static struct semaphore full;
static struct semaphore empty;
static struct semaphore cr;	/* Mutex for critical region */


//device operations 
// - kind of confused what to add and what not to...


//Initialization - for 'insmod'
static int __init fifo_init(void) {
	printk(KERN_INFO "FIFO device loaded\n");
	//Initialize semaphores
	sema_init(&full, 0);
	sema_init(&empty, BUFFER_SIZE);
	sema_init(&cr, 1); //Using this as a mutex
	return 0;
}

//Cleanup - for 'rmmod'
static void __exit fifo_cleanup(void) {
	printk(KERN_INFO "FIFO device unloaded\n");
}


module_init(fifo_init);
module_exit(fifo_cleanup);
