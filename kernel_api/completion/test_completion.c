#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/completion.h>

static int event_flag = 0;
static struct completion comp;
static struct task_struct *my_kthread_1;
static struct task_struct *my_kthread_2;

static int my_thread(void *data)
{
    for (; !kthread_should_stop();) {
        printk("%s, waiting for completion ...\n", __func__);
        wait_for_completion_interruptible(&comp);
        
        printk("%s, somebody just wake me up ...\n", __func__);
        /* Check event flag */
        if (event_flag == 1) {
            printk("%s, event from my thread2\n", __func__);
        }
        if (event_flag == 2) {
            printk("%s, event from init function\n", __func__);
        }
        
        reinit_completion(&comp);
    }
    return 0;
}


static int my_thread2(void *data)
{
    unsigned long loop_count = 0;
    for (; !kthread_should_stop();) {
        if (loop_count == 5) {
            event_flag = 1;
            loop_count = 0;
            
            complete(&comp);
        }
        printk("%s, i'm working ...\n", __func__);
        loop_count++;
        
        msleep(500);
    }
    return 0;
}

static int __init demo_init(void)
{
    init_completion(&comp);
    my_kthread_1 = kthread_run(my_thread, NULL, "my thread");
    my_kthread_2 = kthread_run(my_thread2, NULL, "my thread2");
    
    event_flag = 2;
    complete(&comp);
    return 0;
}

static void __exit demo_exit(void)
{
    kthread_stop(my_kthread_1);
    kthread_stop(my_kthread_2);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_DESCRIPTION("Simple kernel completion usage driver");
MODULE_LICENSE("GPL");
