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
#include <linux/kernel.h>	/* container_of */
#include <linux/list.h>	/* for list */
#include <linux/gpio.h> 	/* for kmalloc */

#define drv_inf(msg) printk(KERN_INFO "%s: "msg ,__func__)
#define drv_dbg(msg) printk(KERN_DEBUG "%s: "msg, __func__)
#define drv_wrn(msg) printk(KERN_WARNING "%s: "msg, __func__)
#define drv_err(msg) printk(KERN_ERR "%s: "msg, __func__)

#define TYPE_DYNAMIC

struct car {
	int door_number;
	char *color;
	char *model;
	struct list_head list;
};

#ifdef TYPE_STATIC
static LIST_HEAD(carlist);
#endif

static __init int demo_init(void)
{
#ifdef TYPE_DYNAMIC
	struct list_head carlist;
	INIT_LIST_HEAD(&carlist);
#endif

	struct car *redcar = kmalloc(sizeof(struct car), GFP_KERNEL);
	struct car *bluecar = kmalloc(sizeof(struct car), GFP_KERNEL);
	struct car *pinkcar = kmalloc(sizeof(struct car), GFP_KERNEL);
	struct car *acar;

	INIT_LIST_HEAD(&redcar->list);
	INIT_LIST_HEAD(&bluecar->list);
	INIT_LIST_HEAD(&pinkcar->list);

	redcar->color = (char *)kmalloc(sizeof(char)*4, GFP_KERNEL);
	redcar->color = "red";
	redcar->model = (char *)kmalloc(sizeof(char)*4, GFP_KERNEL);
	redcar->model = "psv";

	bluecar->color = (char *)kmalloc(sizeof(char)*5, GFP_KERNEL);
	bluecar->color = "blue";
	bluecar->model = (char *)kmalloc(sizeof(char)*4, GFP_KERNEL);
	bluecar->model = "npd";


	pinkcar->color = (char *)kmalloc(sizeof(char)*5, GFP_KERNEL);
	pinkcar->color = "pink";
	pinkcar->model = (char *)kmalloc(sizeof(char)*4, GFP_KERNEL);
	pinkcar->model = "gba";

	list_add(&redcar->list, &carlist);
	list_add(&bluecar->list, &carlist);
	list_add_tail(&pinkcar->list, &carlist);


	list_for_each_entry(acar, &carlist, list){
		printk("--car-- color: %s\tmodel: %s\n", acar->color, acar->model);
	}

	kfree(redcar);
	kfree(bluecar);
	kfree(pinkcar);

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
