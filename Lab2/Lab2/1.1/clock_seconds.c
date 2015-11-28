#include <linux/module.h>	/* Linux Module Manager */
#include <linux/kernel.h>	/* Kernel */
#include <linux/proc_fs.h>	/* For proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */
#include <linux/errno.h>	/* Error numbers and errno handler */
#include <linux/time.h>		/* Kernel time functions */
#include <linux/slab.h>		/* For kmalloc */
#include <linux/rtc.h>		/* For rtc time function */

#define CLOCK_PROC "deeds_clock"

// Time formatting related
#define YEAR_LEN 4
#define MON_DAY_LEN 2
#define SEP_LEN 1
#define HR_MIN_SEC_LEN 2 
#define TS_LEN (YEAR_LEN + 2*MON_DAY_LEN + 3*HR_MIN_SEC_LEN + 5*SEP_LEN + 4)

MODULE_AUTHOR("Group Member Name");
MODULE_DESCRIPTION("Lab 2 - Task 1.1");
MODULE_LICENSE("GPL");
/* TODO: Waiting fr reply in the forum to check what they mean by seconds..*/

//For triggering EOF while reading (End of File)
int eof = 0;

//Read clock
static ssize_t deeds_clock_read(struct file *file, char *user_buf, size_t count, loff_t *ppos){
	char *ts;
	struct timeval time;
	unsigned long local_time;
	struct rtc_time tm;
	
	// Signal EOF if previous read was done.
	if(eof == 1){
		return 0;
	}
		
	ts = kmalloc(sizeof(*ts) * TS_LEN, GFP_KERNEL);
	
	//Fetch current time
	do_gettimeofday(&time);
	
	//Convert to Local Time
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);
	
	//Formatting datetime as per the need of the lab. +1 for hours is to get to CET timezone
	
	//sprintf(ts, "current time: %04d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(ts, "current time: %llu seconds\n", tm.tm_sec);
	
	//Copying to userspace
	copy_to_user(user_buf, ts, TS_LEN);
	
	//Set EOF to 1 for next reading
	eof = 1;
	//Free memory
	kfree(ts);
	
	return TS_LEN;
}

//Write clock call (Not supported)
static ssize_t deeds_clock_write( struct file *file, const char *buf, size_t count, loff_t *ppos ) {
	printk(KERN_ALERT "Clock write operation not supported\n");
	return -EPERM;
}

//Open module (device) call for clock module
static int deeds_clock_open(struct inode * inode, struct file * file) {
	printk(KERN_INFO "Clock file Opened\n");
	eof = 0;
	return 0;	
}

//Releasing clock module (Device) for future operations
static int deeds_clock_release(struct inode * inode, struct file * file) {
	printk(KERN_INFO "Clock file Released.\n");
	return 0;
}

//Standard file Operation structure of  CLOCK_PROC
static struct file_operations clock_fops = {
	.owner =	THIS_MODULE,
	.read =		deeds_clock_read,
	.write =	deeds_clock_write,
	.open =		deeds_clock_open,
	.release =	deeds_clock_release,
};

//Initializing clock module (device) in insmod
static int __init clock_device_init(void) {	
	proc_create(CLOCK_PROC, 0, NULL, &clock_fops);
	return 0;
}

//Removing clock process in rmmod
static void __exit clock_device_cleanup(void) {
	remove_proc_entry(CLOCK_PROC, NULL);
}

module_init(clock_device_init);
module_exit(clock_device_cleanup);
