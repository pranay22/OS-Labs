#include<linux/module.h>
#include<linux/init.h>

#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/fs.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>
#include <uapi/asm-generic/errno-base.h>

#define MAJOR_NUMBER 240 /*major number used for the device. It's not defined in major.h*/
#define DEVICE_CONFIG_NAME "fifo_config"
#define BUFFER_SIZE 8 /*Default size of the fifo buffer*/
#define MAX_BUFFER_SIZE 4096
#define MIN_BUFFER_SIZE 4
#define MAX_CONFIG_ARG_LENGTH 5 // we can only write at most 4 chars to DEVICE_CONFIG_NAME

/*Parameters to control the buffer*/
static char *fifo_buffer=NULL;
static unsigned int buffer_size=BUFFER_SIZE;
/*to see if device is open*/
static unsigned int is_open=0;
/*Positions inside the buffer*/
static unsigned int read_index=-1; /*read to index*/
/*statistics for DEVICE_CONFIG_NAME*/
static unsigned int number_reads=0;
static unsigned int number_writes=0;
static unsigned int number_open=0;

/*device entries*/
// static dev_t fifo0;
// static dev_t fifo1;

int dev_open (struct inode *inode, struct file *filep)
{
	if(is_open)
	{
		printk(KERN_WARNING "Device is open");
		return -EBUSY;
	}
	printk(KERN_DEBUG "Deviced opened\n");
	is_open++;
	number_open++;
	return 0;
}

ssize_t dev_read (struct file *filep, char __user *buf, size_t length, loff_t *offset)
{
	printk(KERN_INFO "Reading...\n");
	if(read_index<0)//buffer empty
	{
		printk(KERN_WARNING "Buffer is empty: cannot read\n");
		return 0; //EOF
	}
	else
	{
		number_reads++;
		length=read_index+1;
		copy_to_user(buf,fifo_buffer,length);
		read_index=-1;
		return length;
	}	
}

ssize_t dev_write (struct file *filep, const char __user *buf, size_t length, loff_t *offset)
{
	int left_space = buffer_size-read_index-1;
	int write_index=read_index+1;
	printk(KERN_INFO "Writing...\n");
	if(left_space<length)//full
	{
		printk(KERN_WARNING "Device is full\n");
		printk(KERN_DEBUG "Left space inside the buffer: %i\n", left_space);
		printk(KERN_DEBUG "length: %li\n",length);
		return -ENOSPC;
	}
	else{
		number_writes++;
		copy_from_user(fifo_buffer+write_index,buf,length);
		read_index+=length;
		return length;
	}
}

int dev_release (struct inode *inode, struct file *filep)
{
	is_open--;
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

static struct proc_dir_entry * device_config_proc = NULL;

int config_read_ctrl=4;
ssize_t config_read (struct file *filep, char __user *buf, size_t length, loff_t *offset)
{
	char msg [128];
	switch(config_read_ctrl)
	{
		case 0:
			config_read_ctrl=4;
			return 0;
		case 4:
			sprintf(msg,"The device has been accessed %i times\n",number_open);
			break;
		case 3:
			sprintf(msg,"Number of reads is %i\nNumber of writes is %i\n", number_reads, number_writes);
			break;
		case 2:
			sprintf(msg, "Buffer size: %i\n", buffer_size);
			break;
		case 1:
			sprintf(msg, "Number of items: %i\n", read_index+1);
			break;
	}
	length=strlen(msg);
	copy_to_user(buf,msg,length);
	config_read_ctrl--;
	return length;	
}
/**
* This function returns the conresponding integer value of the string msg
+ and -EINVAL otherwise
*/
static int parse_to_int (const char * msg, int length)
{
	unsigned int val = 0;
	unsigned int ten=1; 
	int i;
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
			return -EINVAL;
	}
	return -EINVAL;
}

ssize_t config_write (struct file *filep, const char __user *buf, size_t length, loff_t *offset)
{
	char msg [128]; // will contain buffer received from the user
	int new_buffer_size;
	int empty = (read_index==-1 ? 1 : 0);
	printk(KERN_INFO "fifo_config-writing:");
	printk(KERN_INFO "length: %li\n", length);
	if(!is_open &&  empty)
	{
		/*Parsing through the buffer only if the is valid*/
		if(length<=MAX_CONFIG_ARG_LENGTH)
		{
			copy_from_user(msg,buf,length-1);
			printk(KERN_DEBUG "string to be parsed=%s\n",msg);
			new_buffer_size = parse_to_int(msg,length-1);
			printk(KERN_DEBUG "parse_to_int result %i\n", new_buffer_size);
			if(new_buffer_size>=MIN_BUFFER_SIZE && new_buffer_size<=MAX_BUFFER_SIZE)
			{
				printk(KERN_INFO "new buffer value: %i\n", new_buffer_size);
				buffer_size = new_buffer_size;
				kfree(fifo_buffer);
				fifo_buffer = kmalloc(buffer_size, GFP_KERNEL);
				return length;
			}
			else
			{
				printk(KERN_ERR "Invalid arg: Arg value is incorrect\n");
				return -EINVAL;
			} 
		}
		else 
			return -EINVAL;
	}
	printk(KERN_ERR "Either one end of device is open");
	return -EACCES; //permision denied
}
//config file operations
static struct file_operations config_ops={
	.read = config_read,
	.write =config_write
};

static int __init init_driver(void)
{
	/*Regsistring the driver*/
#ifndef DEVICE_NAME
#define DEVICE_NAME "FIFO_CHAR_DRIVER"
#endif
	umode_t mode = 0666 ;
	int reg = register_chrdev(MAJOR_NUMBER, DEVICE_NAME,&device_ops);
	if(reg<0)
	{
		printk(KERN_ERR "Error: registrating driver failed\n");
		return reg;
	}
	/*Alocating memory to fifo_buffer*/
	fifo_buffer = kmalloc(buffer_size,GFP_KERNEL);
	/*Creating config proc entry*/
    device_config_proc = proc_create(DEVICE_CONFIG_NAME,mode,NULL,&config_ops);
    return 0;
}

static void __exit exit_driver(void)
{
	proc_remove(device_config_proc);
	unregister_chrdev(MAJOR_NUMBER,DEVICE_NAME);	
	kfree(fifo_buffer);
    return;
}

module_init(init_driver);
module_exit(exit_driver);

