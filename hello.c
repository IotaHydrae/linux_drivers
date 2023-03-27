#include <linux/init.h>
#include <linux/module.h>

static __init int hello_driver_init(void)
{
	printk("Hello, world!\n");
	return 0;
}

static __exit void hello_driver_exit(void)
{
	printk("Good bye, friend.\n");
}

module_init(hello_driver_init);
module_exit(hello_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("a hello world driver");

