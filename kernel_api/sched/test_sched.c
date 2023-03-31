#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/jiffies.h>

static int __init test_sched_init(void)
{
    printk("test sched driver initializing ...\n");

    set_current_state(TASK_INTERRUPTIBLE);
    //set_current_state(TASK_UNINTERRUPTIBLE);
    printk("schedule out ...\n");
    schedule_timeout(2 * HZ);
    printk("schedule in ...\n");
    return 0;
}

static void __exit test_sched_exit(void)
{
    printk("test sched driver exiting ...\n");
}

module_init(test_sched_init);
module_exit(test_sched_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
