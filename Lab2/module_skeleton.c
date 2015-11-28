#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Lab Solution");
MODULE_LICENSE("GPL");

// this method is executed when reading from the module
static ssize_t gen_module_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{

}

// this method is executed when writing to the module
static ssize_t gen_module_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{

}

// this method is called whenever the module is being used
// e.g. for both read and write operations
static int gen_module_open(struct inode * inode, struct file * file)
{

}

// this method releases the module and makes it available for new operations
static int gen_module_release(struct inode * inode, struct file * file)
{

}

// module's file operations, a module may need more of these
static struct file_operations gen_module_fops = {
	.owner =	THIS_MODULE,
	.read =		gen_module_read,
	.write =	gen_module_write,
	.open =		gen_module_open,
	.release =	gen_module_release,
};

// initialize module (executed when using insmod)
static int __init gen_module_init(void)
{
	printk(KERN_INFO "Skeleton module is being loaded.\n");
	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit gen_module_cleanup(void)
{
	printk(KERN_INFO "Skeleton module is being unloaded.\n");
}

module_init(gen_module_init);
module_exit(gen_module_cleanup);