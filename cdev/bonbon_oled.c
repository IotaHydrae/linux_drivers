/*
	bonbon_oled.c - another driver for oled, char device interface

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
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/module.h>

/*#define BONBON_OLED_IOC_MAGIC		'd'
#define BONBON_OLED_IOCINIT			_IOW(BONBON_OLED_IOC_MAGIC, 0, int)
#define BONBON_OLED_IOCWVALUE		_IOWR(BONBON_OLED_IOC_MAGIC, 1, void *)*/

static int bonbon_oled_nr_devs = 1;
static int bonbon_oled_major;
static int bonbon_oled_minor;
static dev_t bonbon_oled_devid;
int cdev_ok;

static struct cdev *bonbon_cdev;
static struct class *bonbon_oled_class;
static struct device *bonbon_oled_device;

struct bonbon_oled_dev{
	unsigned char vgram[1024];
	struct cdev cdev;
};

static int bonbon_oled_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations bonbon_oled_fops = {
	.owner   = THIS_MODULE,
/*	.open    = bonbon_oled_open,
	.read    = bonbon_oled_read,
	.unlocked_ioctl   = bonbon_oled_ioctl,
	.release = bonbon_oled_release,*/
};

static void bonbon_cleanup(void)
{
	if(bonbon_oled_device)
		device_destroy(bonbon_oled_class, MKDEV(bonbon_oled_major, 0));
	if(bonbon_oled_class)
		class_destroy(bonbon_oled_class);
	if(cdev_ok)
		cdev_del(bonbon_cdev);

	unregister_chrdev_region(MKDEV(bonbon_oled_major, 0), 1);
}

static int bonbon_setup_cdev(struct cdev *dev)
{
	int err;
	printk(KERN_INFO"%s Setting up char dev.\n",__func__);
	cdev_init(dev, &bonbon_oled_fops);
	dev->owner=THIS_MODULE;
	dev->ops = &bonbon_oled_fops;

	err = cdev_add(dev,bonbon_oled_devid,1);

	return err;
}

static __init int bonbon_oled_init(void)
{
	int result = -ENOMEM;

	bonbon_cdev = cdev_alloc();

	printk(KERN_INFO"%s Initing.\n",__func__);

	/* Attempt to assign primary device number  */
	if(bonbon_oled_major){
		bonbon_oled_devid = MKDEV(bonbon_oled_major, bonbon_oled_minor);
		result = register_chrdev_region(bonbon_oled_devid, bonbon_oled_nr_devs, "bonbon_oled");
	}else{
		result = alloc_chrdev_region(&bonbon_oled_devid, 0, bonbon_oled_nr_devs, "bonbon_oled");
		bonbon_oled_major = MAJOR(bonbon_oled_devid);
	}

	/* As for now. No driver operation has been connected to these device numbers. */
	if(result<0){
		printk(KERN_WARNING "%s: can't get major %d\n",__FUNCTION__, bonbon_oled_major);
		goto fail;
	}
	printk(KERN_INFO"major %d.\n",bonbon_oled_major);

	result = bonbon_setup_cdev(bonbon_cdev);
	bonbon_oled_class  = class_create(THIS_MODULE, "bonbon_oled");
	bonbon_oled_device = device_create(bonbon_oled_class, NULL, MKDEV(bonbon_oled_major, 0), NULL, "bonbon_oled");
	if(!result){
		cdev_ok = 1;
	}else{
		goto fail;
	}

	return 0;

fail:
	bonbon_cleanup();
	return result;

}

static __exit void bonbon_oled_exit(void)
{
	printk(KERN_INFO "%s Exiting.\n", __FUNCTION__);
	/*device_destroy(bonbon_oled_class, MKDEV(bonbon_oled_major, 0));
	class_destroy(bonbon_oled_class);
	cdev_del(bonbon_cdev);
	unregister_chrdev_region(MKDEV(bonbon_oled_major, 0), 1);*/
	bonbon_cleanup();
}

module_init(bonbon_oled_init);
module_exit(bonbon_oled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("I2C/ another driver for OLED");
