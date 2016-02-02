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
static int rate = 0;
module_param(rate, int, 0);

#define WORK_QUEUE "consumer_wq"

extern struct data_item* kread(void);

static struct data_item* buffer = NULL;
static struct workqueue_struct* wq = NULL;
static struct delayed_work task;

unsigned int minSize (unsigned int num)
{
    int p = 0;
    while(num)
    {
        p++;
        num/=10;
    }
    return p;
}

static void consume_item(void){
	char * rcvd_msg;

	buffer = kread();
	rcvd_msg = (char*)kmalloc(strlen(buffer->msg)+minSize(buffer->qid)+minSize(buffer->time)+10,GFP_KERNEL);

	char* combined_msg = buffer->msg;
	const char* delimiter = "-";
	char * cp;
	//cp = strdupa(combined_msg);		//making message rewritable
	cp = combined_msg;
	char * instance = strsep(cp, delimiter);
	char * realmsg = strsep(NULL, delimiter);

	sprintf(rcvd_msg,"%s %u,%llu,%s",instance, buffer->qid, buffer->time, realmsg);
	printk(KERN_INFO "Item received : %s\n", rcvd_msg);

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
static DECLARE_DELAYED_WORK(task, consume_item);

static int __init consumer_init(void){
	printk(KERN_INFO "Consumer Initialized\n");
	// Allocate a workqueue, a context to run our tasks.
	wq = alloc_workqueue(WORK_QUEUE, WQ_UNBOUND, 1);
	// Queue the delayed work into workqueue
	queue_delayed_work(wq, &task, HZ / rate);

	return 0;
}

static void __exit consumer_exit(void){
	printk(KERN_INFO "Consumer Exit\n");
	// Cancel delayed task
	cancel_delayed_work(&task);
	// Destroy workqueue
	destroy_workqueue(wq);

	return 0;
}

module_init(consumer_init);
module_exit(consumer_exit);
