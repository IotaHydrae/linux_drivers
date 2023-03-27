#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

static int led_driver_probe(struct platform_device *pdev)
{
	printk(KERN_INFO "into %s\n", __func__);

	return 0;
}

static int led_driver_remove(struct platform_device *pdev)
{
	pr_info("into %s\n", __func__);

	return 0;
}

static struct platform_driver leddrv = {
	.probe = led_driver_probe,
	.remove = led_driver_remove,
	.driver = {
		.name = "led_driver",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(leddrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("a led driver");


