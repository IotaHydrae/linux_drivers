#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#define DRIVER_MAJOR 0
#define DRIVER_NAME "devfs_test"

static devfs_handle_t devfs_handle;

static file_operations devfs_test_fops = {
	.owner = THIS_MODULE,
}

static int __init demo_init(void)
{
	int ret;
	int i;

	ret = register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &devfs_test_fops);
	if (ret < 0) {
		printk("[%s] driver register failed!\n", DRIVER_NAME);
		return ret;
	}

	devfs_handle = devfs_register(NULL, DRIVER_NAME, DEVFS_FL_DEFAULT,
			DRIVER_MAJOR, 0, S_IFCHR | S_IRUSR | SIWUSR,
			&devfs_test_ops, NULL);

	printk(DRIVER_NAME "initialized.\n");

	return 0;
}

static void __exit demo_exit(void)
{
	devfs_unregister(devfs_handle);
	unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_AUTHOR("Iota Hydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");
