#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static char test_str[] = "happy forever!";
static char g_in[64];
static struct proc_dir_entry *parent;

/**
/proc/devices — registered character and block major numbers
/proc/iomem — on-system physical RAM and bus device addresses
/proc/ioports — on-system I/O port addresses (especially for x86 systems)
/proc/interrupts — registered interrupt request numbers
/proc/softirqs — registered soft IRQs
/proc/swaps — currently active swaps
/proc/kallsyms — running kernel symbols, including from loaded modules
/proc/partitions — currently connected block devices and their partitions
/proc/filesystems — currently active filesystem drivers
/proc/cpuinfo — information about the CPU(s) on the system
*/

/*
struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent)

where,

name: The name of the directory that will be created under /proc.
parent: In case the folder needs to be created in a subfolder under
/proc a pointer to the same is passed else it can be left as NULL.
*/

/*
struct proc_dir_entry *proc_create ( const char *name, umode_t mode,
                                    struct proc_dir_entry *parent,
                                const struct file_operations *proc_fops )
<name>: The name of the proc entry
<mode>: The access mode for proc entry
<parent>: The name of the parent directory under /proc.
        If NULL is passed as a parent, the /proc directory
        will be set as a parent.
<proc_fops>: The structure in which the file operations for
            the proc entry will be created.
*/

/*
Note: The above proc_create is valid in the Linux Kernel v3.10 to v5.5.
From v5.6, there is a change in this API. The fourth argument const 
struct file_operations *proc_fops is changed to
const struct proc_ops *proc_ops.
*/

static int proc_open(struct inode *node, struct file *filep)
{
    printk("%s, proc file opend ....\n", __func__);
    return 0;
}

static ssize_t proc_write(struct file *filp_in, const char __user *buf, size_t len, loff_t *ofs)
{
    int rc;
    printk("%s, proc file writing ...\n", __func__);
    rc = copy_from_user(g_in, buf, len);

    printk("%s, from user : %s\n", __func__, g_in);
    return len;
}

static ssize_t proc_read(struct file *filp_in, char __user *buf, size_t len, loff_t *ofs)
{
    int rc;
    printk("%s, proc file reading ... %ld\n", __func__, ARRAY_SIZE(test_str));
    rc = copy_to_user(buf, test_str, ARRAY_SIZE(test_str));
    return 0;
}

static int proc_release(struct inode *node, struct file *filp)
{
    printk("%s, proc file released ...\n", __func__);
    return 0;
}

#if 0
static struct file_operations proc_fops = {
    .open = proc_open,
    .read = proc_read,
    .write = proc_write,
    .release = proc_release,
};

#else
static struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_write = proc_write,
    .proc_release = proc_release,
};
#endif

static int __init procfs_init(void)
{
    parent = proc_mkdir("demo", NULL);
    if (!parent) {
        pr_err("Error creating proc entry!");
        return -1;
    }

    proc_create("demo_proc", 0666, parent, &proc_fops);

    pr_info("Device driver insert .... Done !");
    return 0;
}

static void __exit procfs_exit(void)
{
    proc_remove(parent);

    pr_info("Device driver remove .... Done !");
}

module_init(procfs_init);
module_exit(procfs_exit);
MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_DESCRIPTION("Simple linux procfs usage driver");
MODULE_LICENSE("GPL");
