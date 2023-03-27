#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>

DEFINE_MUTEX(my_mutex);

static __u32 global_data = 0;

static int __init demo_init(void)
{
    mutex_lock(&my_mutex);
    printk(KERN_INFO "Mutex locked\n");
    mutex_unlock(&my_mutex);
    printk(KERN_INFO "Mutex unlocked\n");
    return 0;
}

static void __exit demo_exit(void)
{

}

module_init(demo_init);
module_exit(demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
