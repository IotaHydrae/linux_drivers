#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>

DEFINE_MUTEX(race_lock_1);
static struct mutex *race_lock_2;
static unsigned long loop_count = 0;

static struct task_struct *my_kthread_1;
static struct task_struct *my_kthread_2;


static int kthread_worker_1(void *data)
{
    for (;;) {
        mutex_lock(race_lock_2);
        loop_count++;
        printk("%s, loop count is : %lu\n", __func__, loop_count);
        mutex_unlock(race_lock_2);

        msleep(250);
    }
    return 0;
}

static int kthread_worker_2(void *data)
{
    for (;;) {
        mutex_lock(race_lock_2);
        loop_count++;
        printk("%s, loop count is : %lu\n", __func__, loop_count);
        mutex_unlock(race_lock_2);

        msleep(500);
    }
    return 0;
}

static int __init mutex_demo_init(void)
{
    race_lock_2 = kmalloc(sizeof(*race_lock_2), GFP_KERNEL);
    if (!race_lock_2)
        return -ENOMEM;
        
    mutex_init(race_lock_2);
    
    my_kthread_1 = kthread_create(kthread_worker_1, NULL, "my kthread");
    my_kthread_2 = kthread_create(kthread_worker_2, NULL, "my kthread");

    wake_up_process(my_kthread_1);
    wake_up_process(my_kthread_2);

    return 0;
}

static void __exit mutex_demo_exit(void)
{
    if (race_lock_2)
        kfree(race_lock_2);
}

module_init(mutex_demo_init);
module_exit(mutex_demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
