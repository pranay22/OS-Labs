#include <linux/module.h>	/* Linux Module Manager */
#include <linux/kernel.h>	/* Kernel */
#include <linux/proc_fs.h>	/* For proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */
#include <linux/errno.h>	/* Error numbers and errno handler */
#include <linux/time.h>		/* Kernel time functions */
#include <linux/slab.h>		/* For kmalloc */
#include <linux/rtc.h>		/* For rtc time function */

/* Configuration file for switching clock modes*/

int main ( int argc, char *argv[] ){
	if (argc != 1){
		printk(KERN_ALERT "Not a valid number of arguments\n");
		return -EPERM;
	}
	else{
		int inputNo = argv[0];
		if (inputNo == 0){
			//TODO:: Run clock showing seconds only
		}
		else if (inputNo == 1){
			//TODO:: Run clock showing full date & time
		}
		else{
			printk(KERN_ALERT "Not a valid Input, Choose 0for Seconds, 1 for full date & time\n");
			return -EPERM;
		}
		return 1;
	}
	return 1;
}