#include<linux/time.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/fs.h>
#include<asm/uaccess.h>
#include <asm-generic/errno-base.h> /*For errors*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DEEDS");

//Global dcreadiable for the procfs entry
struct proc_dir_entry *deeds_clock = NULL; //proc deed_clocks
struct proc_dir_entry *config = NULL;// proc deed_clock_config

//These variables are used for the read functions
static int dcread=0;
static int cfread=0;

//variable that contain the config value
static char config_val='0'; //0 is the default value

ssize_t read_deeds_clock(struct file *filep, char __user* buf, size_t length, loff_t *offset)
{
    struct timeval tv;
    char msg[128];
    unsigned long long int current_time;
    size_t temp;  
    do_gettimeofday(&tv);
    current_time = tv.tv_sec;
    sprintf(msg,"current time: %llu\n",current_time);
    printk(KERN_DEBUG "current time: %llu\n",current_time);//log
    temp = strlen(msg);
    if(temp>length)
        temp = length;
    copy_to_user(buf,msg,temp);
    if(dcread)
    {
        dcread =0;
        return 0;       
    }
    else
    {
        dcread = 1;
        return temp;
    }
}

ssize_t read_config(struct file *filep, char __user* buf, size_t length, loff_t *offset)
{
    char msg[128];
    size_t temp=0;
    printk(KERN_DEBUG "reading deeds_clock_config\n");
    sprintf(msg,"current clock format: %c\n", config_val);
    temp = strlen(msg);
    copy_to_user(buf,msg,temp);

    if(cfread)
    {
        cfread=0;
        return 0;
    }
    else{
        cfread=1;
        return temp;
    }
    
}

ssize_t write_config(struct file * filep, const char __user * buf, size_t length, loff_t *offset)
{
    return 0;
}

struct file_operations dc_fops = {
 .read = read_deeds_clock   
};
struct file_operations config_fops = {
  .read = read_config,
  .write = write_config  
};

static int __init init_deeds_clock(void)
{
    //Initialiting mode and proc_ops
    umode_t mode = 0444;//only read is allowed
    deeds_clock = proc_create("deeds_clock",mode,NULL,&dc_fops);
    mode = 0644 ;//only superuser is allowed to write
    config = proc_create("deeds_clock_config",mode,NULL,&config_fops);
    return 0;
}

static void __exit exit_deeds_clock(void)
{
    proc_remove(deeds_clock);
    proc_remove(config);
    return;
}

module_init(init_deeds_clock);
module_exit(exit_deeds_clock);