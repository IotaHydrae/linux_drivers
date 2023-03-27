/*
	demo_container_of.c - template for char device, char device interface

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
#include <linux/module.h>
#include <linux/kernel.h>	/* for container_of */
// #include <linux/gpio.h>		/* for kmalloc */

#define drv_inf(msg) printk(KERN_INFO "%s: "msg ,__func__)
#define drv_dbg(msg) printk(KERN_DEBUG "%s: "msg, __func__)
#define drv_wrn(msg) printk(KERN_WARNING "%s: "msg, __func__)
#define drv_err(msg) printk(KERN_ERR "%s: "msg, __func__)

struct person {
    int age;
    char *name;
};

struct family {
	struct person *father;
	struct person *monther;
	int number_of_suns;
	int salary;
}f;


static __init int demo_init(void)
{
    struct person somebody = {
        .age=12,
	.name="karen",
    };

    struct person *the_person;

    printk("%s\n", somebody.name);

    printk("%lu\n", &somebody);
    printk("%lu\n", &somebody.name);
    char **the_name_ptr = &somebody.name;
    printk("%lu\n", the_name_ptr);

    the_person = container_of(the_name_ptr,struct person,name);

    printk("the name of tmp->name after container_of: %s\n", the_person->name);

    return 0;
}

static __exit void demo_exit(void)
{
    printk(KERN_INFO "%s Exiting.\n", __func__);

}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a driver template for cdev");
