#include <linux/kernel.h>
#include <linux/module.h>

static int __init virt_tty_init(void)
{
    return 0;
}

static void __exit virt_tty_exit(void)
{
}

module_init(virt_tty_init);
module_exit(virt_tty_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
