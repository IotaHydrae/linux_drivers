#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/input.h>

#define DRV_NAME "usb_mouse"

struct usb_mouse_raw {
    char name[128];
    char phys[64];
    
    struct urb                  *urb;
    struct usb_device           *dev;
    struct usb_interface        *intf;
    const struct usb_device_id  *id;
    
    struct input_dev            *indev;
    
    unsigned int                pipe;
    unsigned int                maxp;
    
    void                        *transfer_buffer;
    dma_addr_t                  phy_addr;
    
    int                         interval;
};

/*
 *  This driver is used to test the data transfer progrocess of a usb mouse
 *  Just report the raw data, nothing special.
 *
 *  when a USB deivce pulgged into the computer, the USB controller
 *  will founded it and create a `usb_device`
 */

/*
 * This function used to do the real raw data handle things
 */
static void usb_mouse_raw_irq(struct urb *urb)
{
    struct usb_mouse_raw    *umr  = urb->context;
    struct input_dev        *dev  = umr->indev;
    signed char             *data = umr->transfer_buffer;
    
    switch (urb->status) {
    case 0:     /* success */
        break;
        
    case -ECONNRESET:
    case -ENOENT:
    case -ESHUTDOWN:
        return;
        
    default:
        goto resubmit;
    }
    
    /* handle the raw data */
    printk("%s, data[1] = 0x%x\n", __func__, data[1]);
    input_report_key(dev, KEY_L, data[1] & 0x01);   /* left key pressed */
    input_report_key(dev, KEY_S, data[1] & 0x02);   /* right key pressed */
    input_report_key(dev, KEY_UP, data[1] & 0x10);   /* extra key pressed */
    input_report_key(dev, KEY_DOWN, data[1] & 0x8);   /* side key pressed */
    
    input_report_key(dev, KEY_ENTER, data[1] & 0x04);   /* middle key pressed */
    input_sync(dev);
    
resubmit:
    usb_submit_urb(urb, GFP_ATOMIC);
}

static int usb_mouse_raw_indev_open(struct input_dev *dev)
{
    /* alloc, fill, submit URB */
    struct usb_mouse_raw *umr = input_get_drvdata(dev);
    
    printk("%s, was opend\n", __func__);
    
    usb_submit_urb(umr->urb, GFP_ATOMIC);
    
    return 0;
}

static void usb_mouse_raw_indev_close(struct input_dev *dev)
{
    /* cancel, free URB */
    struct usb_mouse_raw *umr = input_get_drvdata(dev);
    
    usb_kill_urb(umr->urb);
}

/*
 * If this driver got matched when a true device pluging in,
 * this function will be called sooner.
 */
static int usb_mouse_raw_probe(struct usb_interface *intf,
                               const struct usb_device_id *id)
{
    int                             rc;
    struct input_dev                *input_dev;
    struct usb_mouse_raw            *umr;
    struct usb_host_interface       *interface;
    struct usb_endpoint_descriptor  *endpoint_desc;
    /*
     * Here to do the real register operations, you need to do the
     * works list blow :
     *
     * 1.
     */
    printk("a usb mouse has plugged in ..., %s\n", (char *)id->driver_info);
    
    input_dev = devm_input_allocate_device(&intf->dev);
    
    input_dev->dev.parent = &intf->dev;
    input_dev->open  = usb_mouse_raw_indev_open;
    input_dev->close = usb_mouse_raw_indev_close;
    
    set_bit(EV_KEY, input_dev->evbit);
    set_bit(KEY_UP, input_dev->keybit);
    set_bit(KEY_L, input_dev->keybit);
    set_bit(KEY_S, input_dev->keybit);
    set_bit(KEY_DOWN, input_dev->keybit);
    set_bit(KEY_ENTER, input_dev->keybit);
    
    umr = kmalloc(sizeof(*umr), GFP_KERNEL);
    umr->indev = input_dev;
    umr->dev  = interface_to_usbdev(intf);
    umr->intf = intf;
    umr->id   = id;
    
    usb_make_path(umr->dev, umr->phys, sizeof(umr->phys));
    strlcat(umr->phys, "/input0", sizeof(umr->phys));
    
    input_dev->name = "usb_mouse_raw";
    input_dev->phys = umr->phys;
    usb_to_input_id(umr->dev, &input_dev->id);
    
    /* getting pipe, maxp ... */
    interface = intf->cur_altsetting;
    
    if (interface->desc.bNumEndpoints != 1)
        return -ENODEV;
        
    endpoint_desc = &interface->endpoint[0].desc;
    
    if (!usb_endpoint_is_int_in(endpoint_desc))
        return -ENODEV;
        
    umr->interval = endpoint_desc->bInterval;
    umr->pipe = usb_rcvintpipe(umr->dev, endpoint_desc->bEndpointAddress);
    umr->maxp = usb_maxpacket(umr->dev, umr->pipe);
    umr->transfer_buffer = usb_alloc_coherent(umr->dev, 8, GFP_KERNEL,
                                              &umr->phy_addr);
                                              
    umr->urb = usb_alloc_urb(0, GFP_KERNEL);
    usb_fill_int_urb(umr->urb,
                     umr->dev,
                     umr->pipe,
                     umr->transfer_buffer,
                     umr->maxp,
                     usb_mouse_raw_irq,
                     umr, umr->interval);
                     
    umr->urb->transfer_dma = umr->phy_addr;
    umr->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    usb_set_intfdata(intf, input_dev);
    input_set_drvdata(input_dev, umr);
    
    rc = input_register_device(input_dev);
    
    return 0;
}

static void usb_mouse_raw_disconnect(struct usb_interface *intf)
{
    struct input_dev *input_dev = usb_get_intfdata(intf);
    struct usb_mouse_raw *umr =  input_get_drvdata(input_dev);
    
    printk("a usb mouse has plugged out ...\n");
    /* free things inside umr */
    if (umr) {
        usb_kill_urb(umr->urb);
        usb_free_urb(umr->urb);
        usb_set_intfdata(intf, NULL);
        usb_free_coherent(umr->dev, 8, umr->transfer_buffer, umr->phy_addr);
        kfree(umr);
    }
    
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
MODULE_DESCRIPTION("Simple usb mouse raw data driver");
MODULE_LICENSE("GPL");