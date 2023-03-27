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

struct class *eeprom_class;
struct cdev eep_cdev[EEP_NBANK];
static dev_t eeprom_devid;
static int eeprom_major;
static int eeprom_minor;

static int cdev_ok;

static struct device *eeprom_dev;

/* Make your device type here */
struct eeprom_device {
    /* e.g. unsigned char vgram[1024]; */
    unsigned char eeprom_data[256];

    struct cdev cdev;
};

static struct eeprom_device (*eeprom_devp)[EEP_NBANK];

/* PROTOTYPE FOR DEVICE */
static int eeprom_init(void);

/* PROTOTYPE FOR DRIVER */
static void eeprom_cleanup(void);
static int eeprom_setup_cdev(struct eeprom_device (*devp)[]);

static struct file_operations eeprom_fops = {
    .owner = THIS_MODULE,
};

static void eeprom_cleanup(void)
{
	int i;

    if (eeprom_dev){
		for(i=0;i<EEP_NBANK;i++){
			device_destroy(eeprom_class, MKDEV(MAJOR(eeprom_devid), MINOR(eeprom_devid)+i));	
		}
	}
        
    if (eeprom_class)
        class_destroy(eeprom_class);
    if (cdev_ok){
		for(i=0;i<EEP_NBANK;i++){
			cdev_del(&eeprom_devp[i]->cdev);	
		}
	}

    kfree(eeprom_devp);
    unregister_chrdev_region(eeprom_devid, EEP_NBANK);
}

static int eeprom_setup_cdev(struct eeprom_device (*devp)[EEP_NBANK])
{
    int i;
    int err = -1;
    dev_t curr_dev;

    printk(KERN_INFO "%s Setting up char dev.\n", __func__);

    for (i = 0; i < EEP_NBANK; i++) {
        /* devp[i]->cdev = cdev_alloc(); */

        cdev_init(&devp[i]->cdev, &eeprom_fops);
        devp[i]->cdev.owner = THIS_MODULE;

        curr_dev = MKDEV(MAJOR(eeprom_devid), MINOR(eeprom_devid) + i);
        err = cdev_add(&devp[i]->cdev, curr_dev, 1);
        if (err) {
            printk(KERN_INFO "%s: error on setup cdev %d: %d", __func__, i, err);
            return err;
        }

        /* create a node for earch device */
        eeprom_dev = device_create(eeprom_class,
                                   NULL,
                                   curr_dev,
                                   NULL,
                                   EEP_DEVICE_NAME "%d", i);

        if (unlikely(IS_ERR(eeprom_dev))) {
            return PTR_ERR(eeprom_dev);
        }
    }

    return 0;
}

static __init int eeprom_driver_init(void)
{
    int result;

    printk(KERN_INFO "%s driver initing.\n", __func__);

    /* Attempt to assign primary device number  */
    if (eeprom_major) {
        eeprom_devid = MKDEV(eeprom_major, eeprom_minor);
        result = register_chrdev_region(eeprom_devid, EEP_NBANK, EEP_DEVICE_NAME);
    } else {
        result = alloc_chrdev_region(&eeprom_devid, 0, EEP_NBANK, EEP_DEVICE_NAME);
        eeprom_major = MAJOR(eeprom_devid);
    }

    /* As for now. No driver operation has been connected to these device numbers. */
    if (result < 0) {
        printk(KERN_WARNING "%s: can't get major %d\n", __func__, eeprom_major);
        goto fail;
    }

    printk(KERN_INFO "%s: major %d.\n", __func__, eeprom_major);

    eeprom_devp = kmalloc(sizeof(struct eeprom_device) * EEP_NBANK, GFP_KERNEL);
    if (!eeprom_devp) {
        result = -ENOMEM;
        goto fail;
    }
    //memset(eeprom_devp, 0, sizeof(struct eeprom_device) * EEP_NBANK); /* this can be removed when using kzalloc() */

    /* create the class of device, it will be create at `/sys/class` */
    eeprom_class = class_create(THIS_MODULE, EEP_CLASS);
    if (IS_ERR(eeprom_class)) {
        return PTR_ERR(eeprom_class);
    }
    /* self setup function */
    result = eeprom_setup_cdev(eeprom_devp);

    /* create a node for earch device */
    /*eeprom_dev = device_create(eeprom_class, NULL,
    						MKDEV(eeprom_major, 0), NULL, "eeprom");
    if(unlikely(IS_ERR(eeprom_dev))){
    	return PTR_ERR(eeprom_dev);
    }*/

    if (!result) {
        cdev_ok = 1;
    } else {
        goto fail;
    }

    return 0;

fail:
    eeprom_cleanup();
    return result;
}

static __exit void eeprom_driver_exit(void)
{
    printk(KERN_INFO "%s Exiting.\n", __func__);
    /*device_destroy(eeprom_class, MKDEV(eeprom_major, 0));
    class_destroy(eeprom_class);
    cdev_del(bonbon_cdev);
    unregister_chrdev_region(MKDEV(eeprom_major, 0), 1);*/
    eeprom_cleanup();
}

module_init(eeprom_driver_init);
module_exit(eeprom_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a driver template for cdev");
