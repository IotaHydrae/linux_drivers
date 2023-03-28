#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/input.h>

#define DRV_NAME "usb_mouse"

struct usb_mouse_raw {
    struct usb_interface        *intf;
    const struct usb_device_id  *id;
};

/*
 *  This driver is used to test the data transfer progrocess of a usb mouse
 *  Just report the raw data, nothing special.
 *
 *  when a USB deivce pulgged into the computer, the USB controller
 *  will founded it and create a `usb_device`
 */

static int usb_mouse_raw_open(struct input_dev *dev)
{
    /* alloc, fill, submit URB */
    return 0;
}

static void usb_mouse_raw_close(struct input_dev *dev)
{
    /* cancel, free URB */
}

/*
 * If this driver got matched when a true device pluging in,
 * this function will be called sooner.
 */
static int usb_mouse_raw_probe(struct usb_interface *intf,
                           const struct usb_device_id *id)
{
    struct input_dev *input_dev;
    struct usb_mouse_raw *umr;
    /*
     * Here to do the real register operations, you need to do the
     * works list blow :
     *
     * 1.
     */
    printk("a usb mouse has plugged in ...\n");

    input_dev = devm_input_allocate_device(&intf->dev);
    
    input_dev->name = "usb_mouse_raw";
    input_dev->dev.parent = &intf->dev;
    input_dev->open = usb_mouse_raw_open;
    input_dev->close = usb_mouse_raw_close;

    set_bit(EV_KEY, input_dev->evbit);
    set_bit(KEY_A | KEY_L | KEY_ENTER, input_dev->keybit);
    
    umr = kmalloc(sizeof(*umr), GFP_KERNEL);
    umr->intf = intf;
    umr->id = id;
    input_set_drvdata(input_dev, umr);
    usb_set_intfdata(intf, input_dev);


    input_register_device(input_dev);

    return 0;
}

static void usb_mouse_raw_disconnect(struct usb_interface *intf)
{
    struct input_dev *input_dev = usb_get_intfdata(intf);
    struct usb_mouse_raw *umr =  input_get_drvdata(input_dev);

    printk("a usb mouse has plugged out ...\n");

    if (umr)
        kfree(umr);

    input_unregister_device(input_dev);
}

/*
 * Here is used to specify which equipment support
 */
static struct usb_device_id usb_mouse_raw_ids[] = {
    {
        .driver_info = (kernel_ulong_t)"This is a usb mouse",
        USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
                           USB_INTERFACE_SUBCLASS_BOOT,
                           USB_INTERFACE_PROTOCOL_MOUSE)

    },
    { /* KEEP THIS */ }
};
MODULE_DEVICE_TABLE(usb, usb_mouse_raw_ids);

static struct usb_driver usb_mouse_raw_drv = {
    .name       = DRV_NAME,
    .probe      = usb_mouse_raw_probe,
    .disconnect = usb_mouse_raw_disconnect,
    .id_table   = usb_mouse_raw_ids,
};
module_usb_driver(usb_mouse_raw_drv);

MODULE_AUTHOR("IotaHydrae <writeforever@foxmail.com>");
MODULE_DESCRIPTION("Simple usb mouse driver");
MODULE_LICENSE("GPL");