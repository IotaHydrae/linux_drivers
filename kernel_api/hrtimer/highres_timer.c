#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>

/*
struct hrtimer {
	struct timerqueue_node		node;
	ktime_t				_softexpires;
	enum hrtimer_restart		(*function)(struct hrtimer *);
	struct hrtimer_clock_base	*base;
	u8				state;
	u8				is_rel;
	u8				is_soft;
	u8				is_hard;
};
*/

static struct hrtimer my_hr_timer;


static enum hrtimer_restart timer_callback(struct hrtimer *hrt)
{
    printk("%s, timer was called\n", __func__);
    return 0;
}

static int __init highres_timer_init(void)
{
    ktime_t ktime;
    printk("highres timer driver initializing ...\n");

    
    ktime = ktime_set(1, 1e9L);
    hrtimer_init(&my_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hr_timer.function = timer_callback;
    hrtimer_start(&my_hr_timer, ktime, HRTIMER_MODE_REL);

    return 0;
}

static void __exit highres_timer_exit(void)
{
    printk("highres timer driver exiting ...\n");
    hrtimer_cancel(&my_hr_timer);
}

module_init(highres_timer_init);
module_exit(highres_timer_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
