#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/workqueue.h> //for workqueues
#include<linux/slab.h> //for kmalloc()
#include<linux/fs.h> // for filesytem operation
#include "pc.h"

MODULE_AUTHOR("DEEDS");
MODULE_DESCRIPTION("Lab 3 - 2.1 Solution");
MODULE_LICENSE("GPL");

//Module Parameters
static char *name;
module_param(name, charp, S_IRUGO);
static int rate = 0;
module_param(rate, int, 0);
static char *msg_data;
module_param(msg_data, charp, S_IRUGO);

#define WORK_QUEUE "producer_wq"

extern void kwrite(data_item);

static struct data_item* buffer= NULL;
static struct workqueue_struct* wq = NULL;
static struct delayed_work task;

//Produce item
static void produce_item(void){
	char * combined_msg = strcat(msg_data, strcat("-", name)); //prepend instance-name before message
	buffer->msg = combined_msg;
	kwrite(buffer);
	queue_delayed_work(wq, &task, HZ / rate);
}

/**
* Macro DECLARE_DELAYED_WORK - declare a delayed work item
* @name name of declared delayed_work structure
* @fn function to be called in workqueue
*
* This macro declares a struct delayed_work with name @name that executes
* function @fn.
*/
static DECLARE_DELAYED_WORK(task, produce_item);

static int __init producer_init(void){
	printk(KERN_INFO "Producer Initialized\n");
	// Allocate a workqueue, a context to run our tasks.
	wq = alloc_workqueue(WORK_QUEUE, WQ_UNBOUND, 1);
	// Queue the delayed work into workqueue
	queue_delayed_work(wq, &task, HZ / rate);

	return 0;
}

static void __exit producer_exit(void){
	printk(KERN_INFO "Producer Exit\n");
	// Cancel delayed task
	cancel_delayed_work(&task);
	// Destroy workqueue
	destroy_workqueue(wq);
	return 0;
}

module_init(producer_init);
module_exit(producer_exit);
