#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

char tasklet_data[] = "a simple test str";
char dynamic_tasklet_data[] = "this is a dynamic tasklet";

void tasklet_function(struct tasklet_struct *t)
{
    printk(KERN_DEBUG "%s\n", (char *)t->data);
}
DECLARE_TASKLET_DISABLED(my_tasklet, tasklet_function);

void dynamic_tasklet_function(unsigned long data)
{
    printk(KERN_DEBUG "%s\n", (char *)data);
}

struct tasklet_struct *dynamic_tasklet;

static int __init tasklet_demo_init(void)
{
    my_tasklet.data = (unsigned long)tasklet_data;
    tasklet_enable(&my_tasklet);
    tasklet_schedule(&my_tasklet);

    dynamic_tasklet = kmalloc(sizeof(*dynamic_tasklet), GFP_KERNEL);
    if (!dynamic_tasklet) {
        return -ENOMEM;
    }

    tasklet_init(dynamic_tasklet, dynamic_tasklet_function, (unsigned long)dynamic_tasklet_data);
    tasklet_schedule(dynamic_tasklet);

    return 0;
}

static void __exit tasklet_demo_exit(void)
{
    tasklet_kill(&my_tasklet);
    tasklet_kill(dynamic_tasklet);

    if (dynamic_tasklet)
        kfree(dynamic_tasklet);
}

module_init(tasklet_demo_init);
module_exit(tasklet_demo_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_LICENSE("GPL");