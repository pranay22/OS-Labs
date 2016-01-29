#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include  <uapi/asm-generic/errno-base.h>
#include "fifo.h"

#define MAJOR_NUMBER 240 /*major number used for the device. It's not defined in major.h*/
#define DEVICE_CONFIG_NAME "deeds_fifo_stats"
#define MAX_BUFFER_SIZE 4096
#define MIN_BUFFER_SIZE 4
#define MAX_CONFIG_ARG_LENGTH 5 // we can only write at most 4 chars to DEVICE_CONFIG_NAME

MODULE_AUTHOR("DEEDS");
MODULE_DESCRIPTION("Lab 3 - 1.1 Solution");
MODULE_LICENSE("GPL");

/*Parameters to control the buffer*/
static struct data_item **fifo_buffer=NULL;
static unsigned int buffer_size=DEFAULT_SIZE;
/*to see if device is open*/
static unsigned int is_open=0;
/*Positions inside the buffer*/
static unsigned int read_index= DEFAULT_SIZE; /*read to index*/
static unsigned int write_index = 0;
/*statistics for DEVICE_CONFIG_NAME*/
static unsigned int number_reads=0;
static unsigned int number_writes=0;
static unsigned int number_open=0;
static int read_lock = 0;
unsigned int next_qid = 0;
static int stat_read_lock = 0;

struct semaphore full;
struct semaphore empty;
struct semaphore writing;
struct semaphore reading;

/*device entries*/
// static dev_t fifo0;
// static dev_t fifo1;


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

struct data_item* text_to_item(char* text){
	struct data_item* item;
	int tokenCounter;
	int len;
	unsigned int qid;
	unsigned long long time;
	char msg[128];
	char* final_msg;
	char* token;	
	char* separator = ",";
	qid = 0;
	time = 0;
	token = strsep(&text,separator);
	tokenCounter = 0;
	item = (struct data_item*)kmalloc(sizeof(struct data_item), GFP_KERNEL);
	while(token != NULL){
		if (tokenCounter == 1) time = parse_to_llu(token);
		else if (tokenCounter == 2) sprintf(msg, "%s", token);
		token = strsep(&text, separator);
		tokenCounter++;
	}
	len = strlen(msg);
	final_msg = kmalloc(len, GFP_KERNEL);
	sprintf(final_msg, "%s", msg);
	item->qid = next_qid++;
	item->time = time;
	item->msg = final_msg;
	return item;
}

unsigned int get_used_space(void){
	if (read_index < write_index) return write_index - read_index;
	else return read_index - write_index;
}

void add_item(struct data_item* item){
	down(&writing);
	fifo_buffer[write_index] = item;
	write_index++;
	if (write_index == buffer_size) write_index = 0;
	if (read_index == DEFAULT_SIZE) read_index = 0;
	up(&writing);
	down(&full);
	up(&empty);
	number_writes++;
}

struct data_item* get_item(void){
	struct data_item* item;	
	down(&empty);
	up(&full);
	down(&reading);
	item = fifo_buffer[read_index];
	kfree(item->msg);
	kfree(item);
	fifo_buffer[read_index] = NULL;
	read_index++;
	if (read_index == buffer_size) read_index = 0;		
	up(&reading);
	number_reads++;
	return item;
}

int dev_open (struct inode *inode, struct file *filep)
{
	printk(KERN_DEBUG "Deviced opened\n");
	is_open++;
	number_open++;
	return 0;
}

ssize_t dev_read (struct file *filep, char __user *buf, size_t length, loff_t *offset)
{
	char text[128];
	struct data_item* current_item;
	if (!read_lock){
		current_item = get_item();
		sprintf(text,"%u,%llu,%s",current_item->qid, current_item->time, current_item->msg);
		copy_to_user(buf,text,length);
		read_lock = 1;
		return strlen(text);
	}
	else{
		read_lock = 0;
		return 0;
	}
}

ssize_t dev_write (struct file *filep, const char __user *buf, size_t length, loff_t *offset)
{
	char text[length + 1];
	int i;
	struct data_item *item;
	for (i = 0; i < length +1; i++) text[i] = 0;
	copy_from_user(text,buf,length);
	item = text_to_item(text); 
	add_item(item);
	return length;
}

int dev_release (struct inode *inode, struct file *filep)
{
	is_open--;
	read_lock = 0;
	printk(KERN_INFO "Closing...\n");
	return is_open;
}
//device operations
static struct file_operations device_ops = {
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_release	
};

static struct proc_dir_entry * device_stat_proc = NULL;

ssize_t stat_read (struct file *filep, char __user *buf, size_t length, loff_t *offset)
{
	if (stat_read_lock){
		stat_read_lock = 0;
		return 0;
	}
	else{
		char msg [1024];
		int used,i, free, percentage;
		for (i = 0; i < 512; i++) msg[i] = 0;
		used = get_used_space();
		free = buffer_size - used;
		percentage = (used * 100) / buffer_size;
		sprintf(msg,"The device has been accessed %i times\nNumber of reads is %i\nNumber of writes is %i\nBuffer size: %i\nNumber of items: %i\nNumber of free slots: %i\nUtilization: %i%%\nCurrent qid:%i\n",number_open, number_reads, number_writes, buffer_size, used, free, percentage, next_qid);
		length=strlen(msg);
		copy_to_user(buf,msg,length);
		stat_read_lock = 1;
		return length;
	}	
}

//config file operations
static struct file_operations stat_ops={
	.read = stat_read
};

static int __init init_driver(void)
{
	/*Regsistring the driver*/
#ifndef DEVICE_NAME
#define DEVICE_NAME "FIFO_CHAR_DRIVER"
#endif
	int i;	
	umode_t mode = 0666 ;
	int reg = register_chrdev(MAJOR_NUMBER, DEVICE_NAME,&device_ops);
	if(reg<0)
	{
		printk(KERN_ERR "Error: registrating driver failed\n");
		return reg;
	}
	/*Alocating memory to fifo_buffer*/
	fifo_buffer = kmalloc(buffer_size * sizeof (struct data_item),GFP_KERNEL);
	for (i = 0; i < buffer_size; i++) fifo_buffer[i] = NULL;	
	/*Creating config proc entry*/
    device_stat_proc = proc_create(DEVICE_CONFIG_NAME,mode,NULL,&stat_ops);
	/*Initializing semaphores*/
	sema_init(&full, buffer_size);
	sema_init(&empty, 0);
	sema_init(&reading, 1);
	sema_init(&writing, 1);
	printk(KERN_INFO "Installed driver\n");
    return 0;
}

static void __exit exit_driver(void)
{
	int i;	
	proc_remove(device_stat_proc);
	unregister_chrdev(MAJOR_NUMBER,DEVICE_NAME);
	for (i = 0; i < buffer_size; i++){
		if (fifo_buffer[i] != NULL){
			kfree(fifo_buffer[i]->msg);
			kfree(fifo_buffer[i]);
		}
	}
	kfree(fifo_buffer);
    return;
}

EXPORT_SYMBOL_GPL(add_item);
EXPORT_SYMBOL_GPL(get_item);

module_init(init_driver);
module_exit(exit_driver);

