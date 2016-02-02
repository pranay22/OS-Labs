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
#include<linux/proc_fs.h>
#include "fifo.h"

MODULE_AUTHOR("DEEDS");
MODULE_DESCRIPTION("Lab 3 - 1.1 Solution");
MODULE_LICENSE("GPL");

#define WORK_QUEUE "WQ"
#define FIFO_DEV_NAME "deeds_fifo"

static unsigned int size = DEFAULT_SIZE;
module_param(size,uint,0);
static unsigned int major = 0;
module_param(major,uint,0);

static struct semaphore full_sema;
static struct semaphore empty_sema;
static struct semaphore mutex;
static struct semaphore writers_mutex;
static struct semaphore readers_mutex; 
static struct semaphore current_accesses_mutex; //Mutex to protect operations on number of user accesses
static struct semaphore users_mutex;  //Mutex used by user dev_read;
static struct semaphore proc_mutex;

static struct data_item* buffer = NULL;
static struct workqueue_struct* wq = NULL;
static struct proc_dir_entry*  procdir = NULL;
static unsigned int proc_control=1;
static unsigned int access_control = 1;
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
static unsigned int current_accesses; // current accesses user_level
static unsigned int user_reads=0;
static unsigned int user_writes=0;
unsigned int kwrite (struct data_item data)
{
  
    if(down_interruptible(&writers_mutex))
        return -ERESTARTSYS;
    writers++;
    up(&writers_mutex);
    
    if(down_interruptible(&full_sema))
        return -ERESTARTSYS;
    
    if(down_interruptible(&mutex))
        return -ERESTARTSYS;
    data.qid = qid;    //imposing correct qid
    qid++;
    buffer[write_index]=data;
    write_index=(write_index+1)%size;
    total_writes++;
    up(&mutex);
    
    up(&empty_sema);
    
    if(down_interruptible(&writers_mutex))
        return -ERESTARTSYS;
    writers--;
    up(&writers_mutex);
    
    return qid; //returns the value of next qid
}
struct data_item kread (void)
{
    struct data_item data;
    
    down_interruptible(&readers_mutex);
    readers++;
    up(&readers_mutex);
    
    down_interruptible(&empty_sema);
    
    down_interruptible(&mutex);
    data = buffer[read_index];
    read_index=(read_index+1)%size;
    total_reads++;
    up(&mutex);
    
    up(&full_sema);
    
    down_interruptible(&readers_mutex);
    readers--;
    up(&readers_mutex);
    
    return data;
}

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

ssize_t dev_read (struct file *filep, char __user *buff, size_t len, loff_t *offset)
{
    struct data_item data;
    char * msg;
    if(!access_control)
    {
        access_control++;
        up(&users_mutex);
        return 0;
    }
    else{

        data = kread();
        msg = (char*)kmalloc(strlen(data.msg)+minSize(data.qid)+minSize(data.time)+10,GFP_KERNEL);
        sprintf(msg,"%u,%llu,%s",data.qid,data.time,data.msg);
        printk(KERN_CRIT "Read:%s\n",msg);
        if(down_interruptible(&users_mutex))
            return -ERESTARTSYS;
        if(msg == NULL)
        {
            printk(KERN_ERR "Read failed");
            up(&users_mutex);
            return -ENOMEM;
        }
        user_reads++;
        len = strlen(msg);
        copy_to_user(buff,msg,len);
        access_control--;
        return len;       
    }
    return 0;
}

struct data_item parse(char * user_msg, size_t len)
{
    struct data_item data; 
    int comma1 = 0;
    int comma2 = 0;
    int i = 0;
    for(i=0; user_msg[i]!=',' && user_msg[i]>='0' && user_msg[i] <='9' && i < len; i++);
    if(user_msg[i]!=',' || i==len-1)
    {
        data.qid=-1; //Error
        return data;
    }    
    comma1 = i;
    for(i=comma1+1; user_msg[i]!=',' && user_msg[i]>='0' && user_msg[i] <='9' && i < len ; i++);
    if(user_msg[i]!=',' || i==len-1)
    {
        data.qid=-1; //Error
        return data;
    }
    comma2 = i;
    data.qid = 0;
    data.time = 0;
    for(i=0; i<comma1; i++)
        data.qid = data.qid*10+(int)user_msg[i]-'0';
    for(i=comma1+1; i<comma2; i++)
        data.time = data.time*10+(int)user_msg[i]-'0';
        
    data.msg = kmalloc(len-comma2, GFP_KERNEL);
    for(i=0; i<len-comma2-1; i++)
        data.msg[i]=user_msg[comma2+1+i];
    data.msg[i]='\0';
    return data;
     
}

ssize_t dev_write (struct file *filep, char const __user *buff, size_t len, loff_t *offset)
{

    char * copy =  (char*) kmalloc(len*sizeof(char),GFP_KERNEL);
    struct data_item data;
    struct timeval tv;
    printk(KERN_CRIT "copying user's buffer\n");
    copy_from_user(copy,buff,len);
    
    data= parse(copy,len);
    
    printk(KERN_CRIT "Verifying user's message\n");
    do_gettimeofday(&tv);
    if(data.time>tv.tv_sec)
    {
        printk(KERN_ERR "Production time cannot be after write time\n");
        return -EINVAL;
    }
    if(data.qid == -1)
    {
        printk(KERN_ERR "Message from user is not well constructed\n");
        return -EINVAL;
    }
    printk(KERN_CRIT "Write:qid=%d,time=%llu,msg=%s\n",data.qid,data.time,data.msg);
    
    user_writes++;
    kwrite(data);
       
    return len;    
}

int dev_open (struct inode * inode, struct file * filep){
    
    if(down_interruptible(&current_accesses_mutex))
        return -ERESTARTSYS;
    
    current_accesses++;
    up(&current_accesses_mutex);  
    return 0;
}
int dev_release (struct inode *inode, struct file * filep){
    
    if(down_interruptible(&current_accesses_mutex))
         return -ERESTARTSYS;
    current_accesses--;
    up(&current_accesses_mutex);  
     return 0;
}

static struct file_operations fifo_ops = {
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release
};

ssize_t proc_read (struct file *filep, char __user *buff, size_t len, loff_t *offset){
    long int empty=-1;
    long int insertions=-1;
    long int removals =-1;
    long int sequence = -1;
    long int accesses = -1;
    long int stored = -1;
    long int fill = -1;
    if(!proc_control)
    {
        proc_control++;
        up(&proc_mutex);
        return 0;
    }
    else
    {
        char msg [512];
        
        if(down_interruptible(&mutex))
            return -ERESTARTSYS;

        if(write_index<read_index)
            empty = (long int) read_index-write_index;
        else
            empty = (long int) size -write_index+read_index;
        insertions = total_writes;
        removals = total_reads;
        sequence = qid;
        
        up(&mutex);
        
        if(down_interruptible(&current_accesses_mutex))
            return -ERESTARTSYS;
        accesses = current_accesses;
        up(&current_accesses_mutex);
        stored = size -empty;
        fill = stored/size;
        
        sprintf(msg,"Current Fill:\nStored item=%ld, empty places=%ld, fill(percentage)=%lu\nTotal:\ninsertions=%ld, removals=%ld\nSequence number:%ld\nCurrent accesses=%ld\n",stored,empty,fill,insertions,removals,sequence,accesses);
        if(down_interruptible(&proc_mutex))
            return -ERESTARTSYS;
        copy_to_user(buff,msg,strlen(msg));
        proc_control--;
        return strlen(msg);
    }
}

static struct file_operations procops = {
    .read = proc_read
};

static int __init fifo_init(void)
{
    int i = 0;
	int reg = 0;
    printk(KERN_CRIT "Buffer size:%d\n",size);
    buffer = (struct data_item*) kmalloc(size*sizeof(struct data_item), GFP_KERNEL);
    //Initializing the buffer
    for (i=0; i<size; i++)
    {
        buffer[i].qid=0;
        buffer[i].time=0;
        buffer[i].msg = NULL;
    }
    if(!buffer)
    {
        printk(KERN_ERR "Memory Allocation Error\n");
        return -ENOMEM;
    }
    /*Initializing semaphores*/
    sema_init(&mutex,1);
    sema_init(&writers_mutex,1);
    sema_init(&readers_mutex,1);
    sema_init(&empty_sema,0);
    sema_init(&full_sema,size);
    sema_init(&current_accesses_mutex,1);
    sema_init(&users_mutex,1);
    sema_init(&proc_mutex,1);
    /*Intialiting statistic variable*/
    writers=0;
    readers=0;
    total_writes=0;
    total_reads=0;
    current_accesses=0;
    /*Registing a dev driver for user-space interface*/
    reg = register_chrdev(major,FIFO_DEV_NAME,&fifo_ops);
    if(reg)
    {
        printk(KERN_ERR "Registring device failed\n");
        return -EPERM;
    }   
    printk(KERN_CRIT "Major number for fifo: %d\n",major);
    printk(KERN_CRIT "Creating work queue\n");
    wq = alloc_workqueue("deeds_fifo",WQ_UNBOUND,1);
    if(wq == NULL)
        return -EPERM;
    printk(KERN_CRIT "Creating proc entry\n");
    procdir = proc_create("deeds_fifo_stats",444,NULL,&procops);
    
    return 0;
}

static void __exit fifo_exit(void)
{
    int i =0;
    printk(KERN_CRIT "De-loading fifo module\n");
    printk(KERN_CRIT "Destroying workqueue\n");
    destroy_workqueue(wq);
    
    
    printk(KERN_CRIT "Freeing memory of the buffer\n");
    for(i = 0; i<size; i++)
    {
        if(buffer[i].msg != NULL)
            kfree(buffer[i].msg);
    }
        
    kfree(buffer);
    printk(KERN_CRIT "Unregistering the driver\n");
    unregister_chrdev(major, FIFO_DEV_NAME);
    printk(KERN_CRIT "Removing proc entry\n");
    proc_remove(procdir);
    printk(KERN_CRIT "Module fifo exited successfully\n");
    return;
}

module_init(fifo_init);
module_exit(fifo_exit);
//Interface export
EXPORT_SYMBOL(kwrite);
EXPORT_SYMBOL(kread);