#include <linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>

#include<linux/slab.h> //for kmalloc()
#include<linux/fs.h> // for filesytem operation
#include<asm-generic/errno-base.h> //for errors
#include<linux/semaphore.h>	 /* for semaphores & mutex */
#include<linux/moduleparam.h> // for module param
#include<linux/workqueue.h>
#include<linux/time.h>
#include<asm/uaccess.h>
#include "fifo.h"

MODULE_AUTHOR("DEEDS");
MODULE_DESCRIPTION("Lab 3 - 1.1 Solution");
MODULE_LICENSE("GPL");

#define WORK_QUEUE "WQ"

static unsigned int size = DEFAULT_SIZE;
module_param(size,uint,0);
static unsigned int major = 0;
module_param(major,uint,0);

static struct semaphore full_sema;
static struct semaphore empty_sema;
static struct semaphore mutex;
static struct semaphore writers_mutex;
static struct semaphore readers_mutex;

static struct data_item* buffer = NULL;
static struct workqueue_struct* wq = NULL;
static unsigned int read_index =0;
static unsigned int write_index = 0;
static unsigned int qid = 0;

/**
* Statistics
*/
static unsigned int readers; // current number of readers
static unsigned int writers; // current number of writers
static unsigned int total_writes; //Total writes
static unsigned int total_reads; // Total reads 

unsigned int kwrite (struct data_item data)
{
    struct timeval* tv=NULL;
    do_gettimeofday(tv);
    if(data.time>tv->tv_sec)
    {
        printk(KERN_ERR "Production time cannot be after write time");
        return -EINVAL;
    }
    
    down(&writers_mutex);
    writers++;
    up(&writers_mutex);
    
    down(&full_sema);
    
    
    down(&mutex);
    data.qid = qid;    //imposing correct qid
    qid++;
    buffer[write_index]=data;
    write_index=(write_index+1)%size;
    total_writes++;
    up(&mutex);
    
    up(&empty_sema);
    
    down(&writers_mutex);
    writers--;
    up(&writers_mutex);
    
    return qid; //returns the value of next qid
}
struct data_item kread (void)
{
    struct data_item data;
    
    down(&readers_mutex);
    readers++;
    up(&readers_mutex);
    
    down(&empty_sema);
    
    down(&mutex);
    data = buffer[read_index];
    read_index=(read_index+1)%size;
    total_reads++;
    down(&mutex);
    
    up(&full_sema);
    
    down(&readers_mutex);
    readers--;
    up(&readers_mutex);
    
    return data;
}


ssize_t fifo_read (struct file *filep, char __user *buff, size_t len, loff_t *offset)
{
    return 0;
}

ssize_t fifo_write (struct file *filep, char const __user *buff, size_t len, loff_t *offset)
{
    char *copy = (char*) kmalloc(len*sizeof(char),GFP_KERNEL);
    if(!copy)
    {
        printk(KERN_ERR "Mem allocation for copy buffer failed");
        kfree(copy);
        return -ENOMEM;
    }
    unsigned int result = copy_from_user(copy,buff,len);
    if(result != 0)
    {
        printk(KERN_ERR "User's buffer copy failed");
        kfree(copy);
        return -EPERM;
    }
    /*Parsing through the user's buffer*/
    int i = 0;
    int comma1=-1;
    int comma2=-1;
    struct data_item data;
    data.qid = 0;
    data.time = 0;
    
    printk(KERN_DEBUG "Parsing");
    
    for(i=0;i<len && copy[i]!=',' && copy[i]<='9' && copy[i]>='0'; i++);
    if(copy[i]!=',')
    {
        kfree(copy);
        return -EINVAL;
    }
        
    comma1=i;
    for (i=comma1+1; i<len && copy[i]!=',' && copy[i]<='9' && copy[i]>='0';i++);
    if(copy[i]!=',' || i == len)
    {
        kfree(copy);
        return -EINVAL;
    }
    comma2=i;
    
    //calcutating the qid
    for(i=0; i<comma1; i++)
        data.qid = data.qid *10 + (int)(copy[i]-'0');
    //Calculating the time
    for(i=comma1+1; i<comma2; i++)
        data.time = data.time *10 + (int)(copy[i]-'0');
    //copying the msg value
    data.msg = (char*) kmalloc((len-comma2-1)*sizeof(char),GFP_KERNEL);
    if(!data.msg)
    {
        printk(KERN_ERR "Memory allocation for data.msg failed");
        kfree(copy);
        kfree(data.msg);
        return -ENOMEM;
    }
    for(i=comma2+1; i<len; i++)
        data.msg[i-comma2-1]=copy[i];
        
    printk(KERN_INFO "data.qid=%u, data.time=%lu, data.msg=%s", data.qid, data.time, data.msg);
        
    return kwrite(data);
    
}


static struct file_operations fifo_ops = {
    .read = fifo_read,
    .write = fifo_write
};

static int __init fifo_init(void)
{
    buffer = (struct data_item*) kmalloc(size*sizeof(struct data_item), GFP_KERNEL);
    if(!buffer)
    {
        printk(KERN_ERR "Memory Allocation Error");
        return -ENOMEM;
    }
    /*Initializing semaphores*/
    sema_init(&mutex,1);
    sema_init(&writers_mutex,1);
    sema_init(&readers_mutex,1);
    sema_init(&empty_sema,0);
    sema_init(&full_sema,size);
    /*Intialiting statistic variable*/
    writers=0;
    readers=0;
    total_writes=0;
    total_reads=0;
    /*Registing a dev driver for user-space interface*/
#ifndef FIFO_DEV_NAME
#define FIFO_DEV_NAME "FIFO"
#endif
    major = register_chrdev(major,FIFO_DEV_NAME,&fifo_ops);
    if(major<0)
    {
        printk(KERN_ERR "Registring device failed");
        return -EPERM;
    }
    printk(KERN_INFO "Major number for fifo: %d",major);
    // /*Declaring the workqueue*/
    // wq = alloc_workqueue (WORK_QUEUE,WQ_UNBOUND,1);
    // if(!wq)
    // {
    //     printk(KERN_ERR "Work queue error");
    //     return -EPERM;
    // }
    return 0;
}

static void __exit fifo_exit(void)
{
    kfree(buffer);
    return;
}
module_init(fifo_init);
module_exit(fifo_exit);
//Interface export
EXPORT_SYMBOL(kwrite);
EXPORT_SYMBOL(kread);

