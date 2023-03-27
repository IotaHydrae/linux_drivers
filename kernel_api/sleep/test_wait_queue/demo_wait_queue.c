/*
	demo_wait_queue.c - a test driver for wait queue.

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
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#define drv_inf(msg) printk(KERN_INFO "%s: "msg ,__func__)
#define drv_dbg(msg) printk(KERN_DEBUG "%s: "msg, __func__)
#define drv_wrn(msg) printk(KERN_WARNING "%s: "msg, __func__)
#define drv_err(msg) printk(KERN_ERR "%s: "msg, __func__)

#define TYPE_STATIC

#ifdef TYPE_STATIC
#undef TYPE_DYNAMIC
static DECLARE_WAIT_QUEUE_HEAD(my_wq);
static int condition = 0;
#endif

/* 声明工作队列 */
static struct work_struct wrk;

static void work_handler(struct work_struct *work)
{
	printk("Waitqueue module handler %s\n", __func__);
	msleep(5000);
	printk("Wake up the sleeping module\n");
	condition=1;

	/** 
	 * 若条件为true，则唤醒在等待队列中休眠的进程，
	 * 本例中为condition
	 */
	wake_up_interruptible(&my_wq);
}

static __init int demo_init(void)
{
#ifdef TYPE_DYNAMIC
#undef TYPE_STATIC

wait_queue_head_t my_wq;
init_waitqueue_head(&my_wq);

#endif

	printk("Wait queue example\n");

	INIT_WORK(&wrk, work_handler);

	/* 将wrk放入 `kernel-global workqueue` */
	schedule_work(&wrk);

	printk("Going to sleep %s\n", __func__);

	/* 如果条件为false，则阻塞等待队列中的当前任务（进程） */
	wait_event_interruptible(my_wq, condition!=0);

	pr_info("woken up by the work job\n");

    return 0;
}

static __exit void demo_exit(void)
{
    printk(KERN_INFO "waitqueue example cleanup %s Exiting.\n", __func__);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a test driver for wait queue");
