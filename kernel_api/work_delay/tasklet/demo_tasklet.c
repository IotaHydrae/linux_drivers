//
// Created by huazheng on 2022/4/6.
//

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>

char tasklet_data[] = "a simple test str";

void tasklet_function(struct tasklet_struct *t)
{
    printk(KERN_DEBUG "%s\n", (char *)t->data);
}

DECLARE_TASKLET_DISABLED(my_tasklet, tasklet_function);

static int __init demo_init(void)
{
    my_tasklet.data = (unsigned long)tasklet_data;
    tasklet_enable(&my_tasklet);
    tasklet_schedule(&my_tasklet);

    return 0;
}

static void __exit demo_exit(void)
{
    tasklet_kill(&my_tasklet);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");