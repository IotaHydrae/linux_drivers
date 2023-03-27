// SPDX-License-Identifier: GPL-2.0-or-later
/*
	dht11.c - a driver for dht11 with kernel HRT.

	Copyright (C) 2021 Zheng Hua <writeforever@foxmail.com>

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
#include <linux/uaccess.h>

#include "dht11.h"

#define drv_inf(msg) printk(KERN_INFO "%s: "msg ,__func__)
#define drv_dbg(msg) printk(KERN_DEBUG "%s: "msg, __func__)
#define drv_wrn(msg) printk(KERN_WARNING "%s: "msg, __func__)
#define drv_err(msg) printk(KERN_ERR "%s: "msg, __func__)

#define DHT11_NUM 1
#define DHT11_NAME "dht11"
#define DHT11_CLASS "dht11_class"

/* Make your device type here */
struct dht11_device{
	struct mutex lock;
	
	int				temperature;
	int				humidity;

	struct cdev cdev;
};
static struct dht11_device *dht11_devp;


/* static var here */
static int cdev_ok;
/* static int dht11_nr_devs = 1; */
static int dht11_major;
static int dht11_minor;
static dev_t dht11_devid;
static struct class *dht11_class;
static struct device *dht11_dev;

static char kernel_buf[64];
static DEFINE_MUTEX(dht11_mutex);

/* prototype here for device */
static int dht11_init(void);


/* prototype here for driver */
static void dht11_cleanup(void);
static int dht11_setup_cdev(struct cdev *dev);
static int dht11_driver_open(struct inode *inode, struct file *filp);
static ssize_t dht11_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *offset);
static ssize_t dht11_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset);
static long dht11_driver_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int dht11_driver_release (struct inode *inode, struct file *filp);



/* cdev self operations start */
static int dht11_init(void)
{
	int ret=0;
	pr_info("%s", __func__);
	/* configure data pin as output&input */
	
	return ret;
}

static int dht11_deinit(void)
{
	int ret=0;
	pr_info("%s", __func__);
	/* configure data pin as output&input */
	
	return ret;
}

static uint8_t dht11_read_byte(void)
{
	int ret=0;
	pr_info("%s", __func__);

	return ret;
}
/* cdev self operations end */



static struct file_operations dht11_fops = {
	.owner   = THIS_MODULE,
	.open    = dht11_driver_open,
	.read    = dht11_driver_read,
	/*.write   = dht11_driver_write,*/
	.unlocked_ioctl   = dht11_driver_unlocked_ioctl,
	.release = dht11_driver_release,
};


static int dht11_driver_open(struct inode *inode, struct file *filp)
{
	int ret;
	/*e.g. call dht11_init*/
	drv_dbg("device openned.");

	dht11_init();
	
	return 0;
}

static ssize_t dht11_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *offset)
{
	int ret;
	drv_dbg("reading.");
	ret = copy_to_user(buf, kernel_buf, len);
	printk(KERN_DEBUG "%s\n", kernel_buf);
	return ret;
}

/*static ssize_t dht11_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset)
{
	int ret;
	drv_dbg("writing.");
	ret = copy_from_user(kernel_buf, buf, len);
	return ret;
}*/

static long dht11_driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	
	switch(cmd){
	case DHT11_IOCINIT:
		dht11_init();
		break;
	case DHT11_IOCREADBYTE:
		dht11_read_byte();
		break;
	default:
		break;
	}
	return 0;
}

static long dht11_driver_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;

	mutex_lock(&dht11_mutex);
	ret = dht11_driver_ioctl(filp, cmd, arg);
	mutex_unlock(&dht11_mutex);

	return ret;
}


static int dht11_driver_release (struct inode *inode, struct file *filp)
{
	drv_dbg("release.");

	return 0;
}

static void dht11_cleanup(void)
{
	if(dht11_dev)
		device_destroy(dht11_class, MKDEV(dht11_major, 0));
	if(dht11_class)
		class_destroy(dht11_class);
	if(cdev_ok)
		cdev_del(&dht11_devp->cdev);

	kfree(dht11_devp);
	unregister_chrdev_region(MKDEV(dht11_major, 0), 1);
}

static int dht11_setup_cdev(struct cdev *dev)
{
	int err;
	printk(KERN_INFO"%s Setting up char dev.\n",__func__);
	cdev_init(dev, &dht11_fops);
	dev->owner=THIS_MODULE;
	dev->ops = &dht11_fops;

	err = cdev_add(dev,dht11_devid,1);
	if(err){
		printk(KERN_INFO "%s: error on setup cdev : %d",__func__, err);
	}

	return err;
}


static __init int dht11_driver_init(void)
{
	int result;

	printk(KERN_INFO"%s driver initing.\n",__func__);

	/* Attempt to assign primary device number  */
	if(dht11_major){
		dht11_devid = MKDEV(dht11_major, dht11_minor);
		result = register_chrdev_region(dht11_devid, DHT11_NUM, DHT11_NAME);
	}else{
		result = alloc_chrdev_region(&dht11_devid, 0, DHT11_NUM, DHT11_NAME);
		dht11_major = MAJOR(dht11_devid);
	}

	/* As for now. None driver operation has been connected to these device numbers. */
	if(result<0){
		printk(KERN_WARNING "%s: can't get major %d\n",__func__, dht11_major);
		goto fail;
	}
	printk(KERN_INFO "%s: major %d.\n",__func__,dht11_major);

	dht11_devp = kmalloc(sizeof( struct dht11_device), GFP_KERNEL);
	if(!dht11_devp){
		result = -ENOMEM;
		goto fail;
	}
	memset(dht11_devp, 0, sizeof(struct dht11_device));
	
	result = dht11_setup_cdev(&dht11_devp->cdev);
	
	dht11_class  = class_create(THIS_MODULE, DHT11_CLASS);
	if(unlikely(IS_ERR(dht11_class))){
		return PTR_ERR(dht11_class);
	}
	dht11_dev = device_create(dht11_class, NULL, 
							MKDEV(dht11_major, 0), NULL, DHT11_NAME);
	if(unlikely(IS_ERR(dht11_dev))){
		return PTR_ERR(dht11_dev);
	}
	
	if(!result){
		cdev_ok = 1;
	}else{
		goto fail;
	}

	return 0;

fail:
	dht11_cleanup();
	return result;

}

static __exit void dht11_driver_exit(void)
{
	printk(KERN_INFO "%s Exiting.\n", __func__);
	/*device_destroy(dht11_class, MKDEV(dht11_major, 0));
	class_destroy(dht11_class);
	cdev_del(bonbon_cdev);
	unregister_chrdev_region(MKDEV(dht11_major, 0), 1);*/
	dht11_cleanup();
}

module_init(dht11_driver_init);
module_exit(dht11_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a driver template for cdev");
