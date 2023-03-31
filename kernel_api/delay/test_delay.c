#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

/*
 * Though BogoMIPs, this value can be found in /pro/cpuinfo
 * `loops_per_jiffy` it's the variable who stores it.
 *
 * check calibrate_delay() in init/main.c
 */

static int __init test_delay_init(void)
{
    printk("test delay driver initializing ...\n");

    printk("delay out ...\n");
    mdelay(500);
    wake_up_process(current);
    printk("delay in ...\n");
    return 0;
}

static void __exit test_delay_exit(void)
{
    printk("test delay driver exiting ...\n");
}

module_init(test_delay_init);
module_exit(test_delay_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");