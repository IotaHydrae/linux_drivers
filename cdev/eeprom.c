#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/slab.h>

#define EEP_NBANK 8
#define EEP_DEVICE_NAME "eep-mem"
#define EEP_CLASS "eep-class"

struct class *eep_class;
struct cdev eep_cdev[EEP_NBANK];
dev_t dev_num;

static struct file_operations eeprom_fops = {
    .owner   = THIS_MODULE,
};

static int __init eeprom_driver_init(void)
{
    int i;
    dev_t curr_dev;

    alloc_chrdev_region(&dev_num, 0, EEP_NBANK, EEP_DEVICE_NAME);

    eep_class = class_create(THIS_MODULE, EEP_CLASS);

    for(i=0; i<EEP_NBANK; i++) {
        cdev_init(&eep_cdev[i], &eeprom_fops);
        eep_cdev[i].owner = THIS_MODULE;

        curr_dev = MKDEV(MAJOR(dev_num), MINOR(dev_num)+i);

        cdev_add(&eep_cdev[i], curr_dev, 1);

        device_create(eep_class,
                      NULL,
                      curr_dev,
                      NULL,
                      EEP_DEVICE_NAME "%d", i);
    }

    return 0;
}

static __exit void eeprom_driver_exit(void)
{
    int i;

    printk(KERN_INFO "%s Exiting.\n", __func__);

    for(i=0; i<EEP_NBANK; i++) {

        device_destroy(eep_class, MKDEV(MAJOR(dev_num), MINOR(dev_num)+i));

    }


    class_destroy(eep_class);
    for(i=0; i<EEP_NBANK; i++) {

        cdev_del(&eep_cdev[i]);

    }

    unregister_chrdev_region(dev_num, EEP_NBANK);


}

module_init(eeprom_driver_init);
module_exit(eeprom_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a driver template for cdev");