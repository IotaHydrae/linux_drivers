#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>

DEFINE_SPINLOCK(demo_spinlock);

spinlock_t dynamic_demo_spinlock;

/*
 * Spinlock can be used in those conditions :
 *
 * - locking between user context
 * - locking between bottom halves
 * - locking between from user context and bottom halves
 * - locking between hard irq and bottom halves
 * - locking between hard irqs
 */

static unsigned long loop_count = 0;
static struct task_struct *my_kthread_1;
static struct task_struct *my_kthread_2;


static int kthread_worker_1(void *data)
{
    for (;!kthread_should_stop();) {
        spin_lock(&dynamic_demo_spinlock);
        loop_count++;
        printk("%s, loop count is : %lu\n", __func__, loop_count);
        spin_unlock(&dynamic_demo_spinlock);

        msleep(250);
    }
    return 0;
}

static int kthread_worker_2(void *data)
{
    for (;!kthread_should_stop();) {
        spin_lock(&dynamic_demo_spinlock);
        loop_count++;
        printk("%s, loop count is : %lu\n", __func__, loop_count);
        spin_unlock(&dynamic_demo_spinlock);

        msleep(500);
    }
    return 0;
}


static int __init spinlock_demo_init(void)
{
    spin_lock_init(&dynamic_demo_spinlock);

    my_kthread_1 = kthread_run(kthread_worker_1, NULL, "my kthread");
    my_kthread_2 = kthread_run(kthread_worker_2, NULL, "my kthread");

    return 0;
}

static void __exit spinlock_demo_exit(void)
{
    kthread_stop(my_kthread_1);
    kthread_stop(my_kthread_2);
}

module_init(spinlock_demo_init);
module_exit(spinlock_demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
