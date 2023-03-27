#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static struct timer_list my_timer;

/*
 * In old kernel, callback of timer should be like this:
 */
// static void timer_callback(unsigned long data)
// {

// }

static void timer_callback(struct timer_list *t)
{
    printk("%s, timer was called\n", __func__);
    mod_timer(&my_timer, jiffies_64 + msecs_to_jiffies(500));
}

static void static_timer_callback(struct timer_list *t)
{
    printk("%s, timer was called\n", __func__);
}
DEFINE_TIMER(static_timer, static_timer_callback);

static int __init basic_timer_init(void)
{
    printk("basic timer driver initializing ...\n");

    /*
     * In old kernel, timer setup should be like this:
     */
    // setup_timer(&my_timer, timer_callback, 0);

    timer_setup(&my_timer, timer_callback, 0);

    mod_timer(&my_timer, jiffies_64 + msecs_to_jiffies(500));

    mod_timer(&static_timer, jiffies_64 + msecs_to_jiffies(500));
    return 0;
}

static void __exit basic_timer_exit(void)
{
    printk("basic timer driver exiting ...\n");
    del_timer(&my_timer);
}

module_init(basic_timer_init);
module_exit(basic_timer_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
