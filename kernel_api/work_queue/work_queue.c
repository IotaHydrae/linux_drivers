#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>

#include <linux/workqueue.h>

#define DRV_NAME "test_work_queue"

static int work_queue_open(struct inode *node, struct file *filep)
{
    printk("%s, work_queue file opend ....\n", __func__);
    return 0;
}

static ssize_t work_queue_write(struct file *filp_in, const char __user *buf, size_t size,
                                loff_t *ofs)
{
    printk("%s, work_queue file writing ...\n", __func__);
    return size;
}

static ssize_t work_queue_read(struct file *filp_in, char __user *buf, size_t size,
                               loff_t *ofs)
{
    printk("%s, work_queue file reading ...\n", __func__);
    return 0;
}

static int work_queue_release(struct inode *node, struct file *filp)
{
    printk("%s, work_queue file released ...\n", __func__);
    return 0;
}

static const struct file_operations work_queue_fops = {
    .owner = THIS_MODULE,
    .open = work_queue_open,
    .read = work_queue_read,
    .write = work_queue_write,
    .release = work_queue_release,
};

static struct miscdevice test_work_queue_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DRV_NAME,
    .fops = &work_queue_fops,
};

// static int wait_function(void *priv)
// {
//     for (;;) {
//         pr_info("waiting for event ...");
//     }
//     do_exit(0);
//     return 0;
// }

/* Static method */
static void work_queue_test_handler(struct work_struct *w)
{
    printk("i'm a bad people...\n");
}
DECLARE_WORK(test_work, work_queue_test_handler);

static void dynamic_work_queue_test_handler(struct work_struct *w)
{
    printk("dynamic, i'm a good people...\n");
}

struct work_struct default_work;
struct workqueue_struct *keventd_wq;

static int __init work_queue_init(void)
{
    printk("hello\n");

    misc_register(&test_work_queue_miscdev);
    schedule_work(&test_work);

    INIT_WORK(&default_work, dynamic_work_queue_test_handler);

    printk("creating workqueue myself...\n");
    keventd_wq = create_workqueue("events");

    printk("queue workqueue myself...\n");
    queue_work(keventd_wq, &default_work);

    printk("flush workqueue myself...\n");
    flush_workqueue(keventd_wq);

    return 0;
}

static void __exit work_queue_exit(void)
{
    printk("goodbye, kind friend.\n");
    schedule_work(&test_work);
    queue_work(keventd_wq, &default_work);

    destroy_workqueue(keventd_wq);
    misc_deregister(&test_work_queue_miscdev);
}

module_init(work_queue_init);
module_exit(work_queue_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
