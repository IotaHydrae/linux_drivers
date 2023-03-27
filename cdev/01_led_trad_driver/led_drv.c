/*
	led.c - another driver for led, char device interface

	Copyright (C) 2021-22 Zheng Hua <writeforever@foxmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/uaccess.h> /*for copy_from/to_user*/

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

static int led_nr_devs = 1;
static int led_major;
static int led_minor;
static dev_t led_devid;
int cdev_ok;

static struct cdev *led_cdev;
static struct class *led_class;
static struct device *led_device;

/*Use led2 at gpio5_3, the pin named SNVS_TAMPER3*/
#define GROUP_PIN(g,p)(g<<16 | p)
#define GROUP(gp)(gp >> 16)
#define PIN(gp)(gp & 0xffff)
#define IMX6ULL_GPIO5_PINn(n)(5<<16 | n)

static unsigned int led_pin = IMX6ULL_GPIO5_PINn(3);


/*
	CCM:	Clock Control Module
	Well, the GPIO5 in IMX6ULL(not IMX6UL) was automatically enabled.
	So we shouldn't set it manually.
*/

/*
	IOMUX:	IOMUX Controller
	The SNVS_TAMPER3 was Controled by
	SW_MUX_CTL_PAD_SNVS_TAMPER3 SW MUX Control Register
	Address: 229_0000h base + 14h offset = 229_0014h
	write 0b101 to it to enable ALT5 mode(ALT5 mode in only 
	valid when TAMPER_PIN is used as GPIO)
*/
static volatile unsigned int *sw_mux_ctl_pad_snvs_tamper3;

/*
	GPIO: 
	Here two registers we will use.
	1.GPIO direction register (GPIOx_GDIR) at (Base + 4h offset)
	2.GPIO data register (GPIOx_DR)	at (Base +0h offset)
	The Base in this e.g. was 20A_C000, it related with the group 
	of GPIO, you should check the 1357th page of IMX6ULLRM.pdf to determine it.
	
*/
static volatile unsigned int *gpio5_gdir;
static volatile unsigned int *gpio5_dr;


static int led_open(struct inode *inode, struct file *filp)
{
	unsigned int gpio_pin = PIN(led_pin);
	printk(KERN_INFO "led_open\n");
	/*ccm init do nothing*/

	/*Set snvs_tamper3 in ALT5 mode*/	
	sw_mux_ctl_pad_snvs_tamper3 = ioremap(0x2290014, 4);
	if(sw_mux_ctl_pad_snvs_tamper3){
		printk("ioremap(0x2290014) = %x\n", sw_mux_ctl_pad_snvs_tamper3);
		*sw_mux_ctl_pad_snvs_tamper3 = 5;	
	}else{
		return -EINVAL;
	}


	/*Set GPIO5_3 as OUTPUT mode*/
	gpio5_gdir = ioremap(0x20ac004, 4);
	if(gpio5_gdir){
		printk("ioremap(0x20ac004) = %x\n", gpio5_gdir);
		*gpio5_gdir |=  (1<<3);

	}else{
		return -EINVAL;
	}

	/*ioremap the gpio5 dat register*/
	gpio5_dr = ioremap(0x20ac000, 4);
	if(gpio5_dr){
		printk("ioremap(0x20ac000) = %x\n", gpio5_dr);
	}else{
		return -EINVAL;
	}
	return 0;
}

static ssize_t led_write(struct file *filp_in, const char __user *buf, size_t size, loff_t *ofs)
{
	int ret;
	unsigned char val = 0;
	
	ret = copy_from_user(&val, buf, 1);
	printk("res: %d\n", ret);
	printk(KERN_INFO "led %s\n",val==1?"on":"off");
	if(val){
		*gpio5_dr &= ~(1<<3);
	}else{

		*gpio5_dr |=  (1<<3);
	}
	
	return 1;
}

/*callback when all copy was closed*/
static int led_release (struct inode *node, struct file *filp)
{
	/*unmap here*/
	iounmap(sw_mux_ctl_pad_snvs_tamper3);
	iounmap(gpio5_gdir);
	iounmap(gpio5_dr);
	return 0;
}

static struct file_operations led_fops = {
	.owner   = THIS_MODULE,
	.open    = led_open,
	.write   = led_write,
	.release = led_release,
};

static void led_cleanup(void)
{
	if(led_device)
		device_destroy(led_class, MKDEV(led_major, 0));
	if(led_class)
		class_destroy(led_class);
	if(cdev_ok)
		cdev_del(led_cdev);

	unregister_chrdev_region(MKDEV(led_major, 0), 1);
}

static int led_setup_cdev(struct cdev *dev)
{
	int err;
	printk(KERN_INFO"%s Setting up char dev.\n",__func__);
	cdev_init(dev, &led_fops);
	dev->owner=THIS_MODULE;
	dev->ops = &led_fops;

	err = cdev_add(dev,led_devid,1);

	return err;
}

static __init int led_init(void)
{
	int result = -ENOMEM;

	led_cdev = cdev_alloc();

	printk(KERN_INFO"%s Initing.\n",__func__);

	/* Attempt to assign primary device number  */
	if(led_major){
		led_devid = MKDEV(led_major, led_minor);
		result = register_chrdev_region(led_devid, led_nr_devs, "led");
	}else{
		result = alloc_chrdev_region(&led_devid, 0, led_nr_devs, "led");
		led_major = MAJOR(led_devid);
	}

	/* As for now. No driver operation has been connected to these device numbers. */
	if(result<0){
		printk(KERN_WARNING "%s: can't get major %d\n",__FUNCTION__, led_major);
		goto fail;
	}
	printk(KERN_INFO"major %d.\n",led_major);

	result = led_setup_cdev(led_cdev);
	led_class  = class_create(THIS_MODULE, "led");
	led_device = device_create(led_class, NULL, MKDEV(led_major, 0), NULL, "led");
	if(!result){
		cdev_ok = 1;
	}else{
		goto fail;
	}

	return 0;

fail:
	led_cleanup();
	return result;

}

static __exit void led_exit(void)
{
	printk(KERN_INFO "%s Exiting.\n", __FUNCTION__);
	if(led_device)
		device_destroy(led_class, MKDEV(led_major, 0));
	if(led_class)
		class_destroy(led_class);
	if(cdev_ok)
		cdev_del(led_cdev);

	unregister_chrdev_region(MKDEV(led_major, 0), 1);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("I2C/ another driver for OLED");

