#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>

/* these macros are available for different wait uses.
 * wait_event
 * wait_event_timeout
 * wait_event_cmd
 * wait_event_interruptible
 * wait_event_interruptible_timeout
 * wait_event_killable
 */

/*
 * these macros are available for different wait uses.
 * wake_up
 * wake_up_all
 * wake_up_interruptible
 * wake_up_sync
 * wake_up_interruptible_sync
 */

static struct task_struct *wait_thread;

/* 
 * Declare a wait queue
 * static method 
 */
DECLARE_WAIT_QUEUE_HEAD(s_wq);
int wait_queue_flag = 0;

static int wait_queue_open(struct inode *node, struct file *filep)
{
    printk("%s, wait_queue file opend ....\n", __func__);
    return 0;
}

static ssize_t wait_queue_write(struct file *filp_in, const char __user *buf, size_t size, loff_t *ofs)
{
    printk("%s, wait_queue file writing ...\n", __func__);
    return size;
}

static ssize_t wait_queue_read(struct file *filp_in, char __user *buf, size_t size, loff_t *ofs)
{
    printk("%s, wait_queue file reading ...\n", __func__);
    wait_queue_flag = 1;
    wake_up_interruptible(&s_wq);
    return 0;
}

static int wait_queue_release(struct inode *node, struct file *filp)
{
    printk("%s, wait_queue file released ...\n", __func__);
    return 0;
}

static int wait_function(void *priv)
{
    for (;;) {
        pr_info("waiting for event ...");
        wait_event_interruptible(s_wq, wait_queue_flag != 0);
        if (wait_queue_flag == 2) {
            pr_info("event came from EXIT function\n");
            return 0;
        }
        pr_info("event came from read function\n");
        wait_queue_flag = 0;
    }
    do_exit(0);
    return 0;
}

static const struct file_operations wait_queue_fops = {
    .owner = THIS_MODULE,
    .open = wait_queue_open,
    .read = wait_queue_read,
    .write = wait_queue_write,
    .release = wait_queue_release,
};

static struct miscdevice wait_queue_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "test_wait_queue",
    .fops = &wait_queue_fops,
};

static int __init demo_init(void)
{
    /* 
    * Declare a wait queue
    * Dynamic method 
    */
    wait_queue_head_t d_wq;
    init_waitqueue_head(&d_wq);

    misc_register(&wait_queue_miscdev);

    wait_thread = kthread_create(wait_function, NULL, "waiting thread");
    if (wait_thread) {
        pr_info("thread created successfully!\n");
        wake_up_process(wait_thread);
    }
    return 0;
}

static void __exit demo_exit(void)
{
    pr_info("living is good, but i should go ...\n");
    wait_queue_flag = 2;
    wake_up_interruptible(&s_wq);
    misc_deregister(&wait_queue_miscdev);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
