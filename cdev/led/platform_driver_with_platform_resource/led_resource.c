#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

struct my_gpios {
	int reset_gpios;
	int dc_gpios;
};

static struct my_gpios needed_gpios = {
	.reset_gpios = 12,
	.dc_gpios    = 11,
};

static struct resource needed_resources[] = {
	[0] = {
		.start = 0x00,
		.end   = 0x01,
		.flags = IORESOURCE_MEM,
		.name = "mem1",
	},
	[1] = {
		.start = 0x02,
		.end   = 0x03,
		.flags = IORESOURCE_MEM,
		.name  = "mem2",
	},
	[2] = {
		.start = 0x04,
		.end   = 0x04,
		.flags = IORESOURCE_IRQ,
		.name  = "mc",
	},
};


static struct platform_device led_device = {
	.name = "led_platform_device",
	.id   = 0,
	.dev  = {
		.platform_data = &needed_gpios,
	},
	.resource      = needed_resources,
	.num_resources = ARRAY_SIZE(needed_resources),
};

static __init int led_resource_init(void)
{
	printk("led device registered.\n");
	platform_device_register(&led_device);
	return 0;
}

static __exit void led_resource_exit(void)
{
	printk("led device unregistered.\n");
	platform_device_unregister(&led_device);
}

module_init(led_resource_init);
module_exit(led_resource_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("a led platform device");
