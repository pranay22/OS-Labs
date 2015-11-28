#include <linux/module.h>	/* Linux Module Manager */
#include <linux/kernel.h>	/* Kernel */
#include <linux/proc_fs.h>	/* For proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */
#include <linux/errno.h>	/* Error numbers and errno handler */
#include <linux/time.h>		/* Kernel time functions */
#include <linux/sched.h>	/* Is a dependency for wait.h */
#include <linux/slab.h>		/* For kmalloc */
#include <linux/rtc.h>		/* For rtc time function */
#include <linux/slab.h>		/* For kmalloc */
#include <linux/wait.h>		/* For Sleep - wakeup */

#define FIFO_NAME "fifo"
#define FIFO_MAJOR_NUM 240

#define BUFFER_MAX_SIZE 10
#define FIFO_MAX_COUNT 4

//Sleep wakeup queue
static DECLARE_WAIT_QUEUE_HEAD(wq);

char buffer[FIFO_MAX_COUNT/2][BUFFER_MAX_SIZE];
int open[FIFO_MAX_COUNT];
int curpos[FIFO_MAX_COUNT/2];

MODULE_AUTHOR("Group Work");
MODULE_DESCRIPTION("Task 2.1");
MODULE_LICENSE("GPL");

//Reading from FIFO
static ssize_t fifo_driver_read(struct file *file, char *user_buf, size_t count, loff_t *ppos) {
	// Get minor number to see if read operation is supported 
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	int bytes_read = 0;
	int fifo;
	// Only odd number FIFOs are read ends
	if(minor%2 == 0){
		printk(KERN_ALERT "Write only fifo end. Read not permitted\n");
		return -EACCES;
	}
	// Identify the fifo number
	fifo = minor/2;
	// Check if fifo is empty
	if(curpos[fifo] == 0) {
		// Sleep if the corresponding writer is writing
		if(open[minor-1]){
			printk(KERN_INFO "Fifo %d empty. Reader sleeping\n", fifo);
			wait_event_interruptible(wq, open[minor-1] == 0 || curpos[fifo] != 0);			
		}
		// If fifo empty even after wake up
		if(curpos[fifo] == 0){
			printk(KERN_INFO "Fifo %d is empty\n", fifo);
			return 0;
		}
	}
	//Read FIFO data
	bytes_read = curpos[fifo];
	copy_to_user(user_buf, buffer[fifo], sizeof(int)*curpos[fifo]);
	curpos[fifo] = 0;
	//Wake up waiting processes (if any)
	wake_up_interruptible(&wq)
	return bytes_read;
}

//Writing to FIFO
static ssize_t fifo_driver_write( struct file *file, const char *buf, size_t count, loff_t *ppos ) {
	// Get minor number to see if write operation is supported
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	int bytes_written = 0;
	int fifo;
	//Only even number FIFOs are write ends
	if(minor%2 != 0){
		printk(KERN_ALERT "Read only FIFO end. Write not permitted\n");
		return -EACCES;
	}
	//Identify the fifo number
	fifo = minor/2;
	while(bytes_written != count){
		//Check if the buffer is full
		if(curpos[fifo] != BUFFER_MAX_SIZE){					
			buffer[fifo][curpos[fifo]] = *buf++;
			curpos[fifo]++;
			bytes_written++;
			//Signal readers waiting on this
			wake_up_interruptible(&wq);
		} 
		//When buffer is full, sleep and wait for reader.
		else{
			printk(KERN_INFO "Fifo %d full. Writer sleeping\n", fifo);
			wait_event_interruptible(wq, curpos[fifo] != BUFFER_MAX_SIZE);
		}	
	}
	printk(KERN_INFO "FIFO write done\n");
	return bytes_written;
}

//Opening FIFO
static int fifo_driver_open(struct inode * inode, struct file * file) {
	//Fetch minor number and open it
	int minor = MINOR(inode->i_rdev);
	open[minor] = 1;
	printk(KERN_INFO "FIFO opened\n");
	return 0;	
}

//Releasing FIFO
static int fifo_driver_release(struct inode * inode, struct file * file) {
	//Fetch minor number and close it
	int minor = MINOR(inode->i_rdev);
	open[minor] = 0;
	printk(KERN_INFO "FIFO released\n");
	//Trigger anyone waiting on this
	wake_up_interruptible(&wq);
	return 0;
}

//File operation structure - FIFO
static struct file_operations fifo_fops = {
	.owner =	THIS_MODULE,
	.read =		fifo_driver_read,
	.write =	fifo_driver_write,
	.open =		fifo_driver_open,
	.release =	fifo_driver_release,
};

// Initializing FIFO driver on insmod
static int __init fifo_device_init(void) {	
	int major;
	//Register character driver
	major = register_chrdev(FIFO_MAJOR_NUM, FIFO_NAME, &fifo_fops);
	if(major < 0) {
		printk(KERN_ALERT "Error registering char driver %d\n", major);
		return major;
	}
	return 0;
}

//Removing FIFO driver on rmmod
static void __exit fifo_device_cleanup(void) {
	unregister_chrdev(FIFO_MAJOR_NUM, FIFO_NAME);
}

module_init(fifo_device_init);
module_exit(fifo_device_cleanup);
