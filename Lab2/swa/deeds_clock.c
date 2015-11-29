#include<linux/time.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/fs.h>
#include<asm/uaccess.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DEEDS");

//Global variable for the procfs entry
struct proc_dir_entry *deeds_clock = NULL;
static unsigned long long int current_time;
int var=0;

ssize_t read_deeds_clock(struct file *filep, char __user* buf, size_t length, loff_t *offset)
{
    struct timeval tv;
    char msg[128];
    size_t temp;   
    //printk(KERN_INFO "inside read_deeds_clock\n");
    //printk(KERN_INFO "length: %lu; offset: %llu\n", length, *offset);
    do_gettimeofday(&tv);
    current_time = tv.tv_sec;
    sprintf(msg,"current time: %llu\n",current_time);
    printk(KERN_WARNING "current time: %llu\n",current_time);
    temp = strlen(msg);
    if(temp>length)
        temp = length;
    copy_to_user(buf,msg,temp);
    if(var)
    {
        var =0;
        return 0;       
    }
    else
    {
        var = 1;
        return temp;
    }
}

struct file_operations proc_fops = {
 .read = read_deeds_clock   
};

static int __init init_deeds_clock(void)
{
    //Initialiting mode and proc_ops
    umode_t mode = 0;
    deeds_clock = proc_create("deeds_clock",mode,NULL,&proc_fops);
    return 0;
}

static void __exit exit_deeds_clock(void)
{
    proc_remove(deeds_clock);
    return;
}

module_init(init_deeds_clock);
module_exit(exit_deeds_clock);