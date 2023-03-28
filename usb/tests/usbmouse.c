/**
 * @file usbmouse.c
 * @author IotaHydrae (writeforever@foxmail.com)
 * @brief find the usb mouse and read data
 * @version 0.1
 * @date 2023-02-28
 *
 * MIT License
 *
 * Copyright 2022 IotaHydrae(writeforever@foxmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <libusb-1.0/libusb.h>

int endpoint_out;
int endpoint_in;
int interface_n;
int num_devices;
int packet_size;
libusb_device *dev, * *devs;
libusb_device_handle *dev_handle;

struct usb_mouse {
    int ep_in;
    int ep_out;
    int interface;

    libusb_device *dev;
    libusb_device_handle *dev_handle;

    int packet_size;
    unsigned char *packet_data;
};

enum usb_device_result {
    dr_vidpid_ok,
    dr_vidpid_unknown,
    dr_error,
};

#define VENDOR_ID_RAZER     0x1532
#define PRODUCT_ID_MOUSE    0x006e

static int usb_mouse_probe(libusb_device *dev, libusb_device_handle **dev_handle)
{
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;

    *dev_handle = NULL;

    int rc = libusb_get_device_descriptor(dev, &desc);
    if (rc) {
        printf("Failed to get device descriptor\n");
        return rc;
    }

    rc = libusb_get_active_config_descriptor(dev, &config);
    if (rc) {
        printf("Failed to read config descriptor\n");
        return rc;
    }

    /* vendor and product id specify */
    if (desc.idVendor != VENDOR_ID_RAZER) {
        printf("Unrecognizable vendor 0x%04x!\n", desc.idVendor);
        return dr_vidpid_unknown;
    }

    switch (desc.idProduct) {
    case PRODUCT_ID_MOUSE:
        printf("This is the USB device we want 0x%04x:0x%04x\n", desc.idVendor, desc.idProduct);
        break;
    default:
        printf("Unrecognizable product! 0x%04x\n", desc.idProduct);
        return dr_vidpid_unknown;
    }

    /* interface specify */
    for (int interface = 0; interface < config->bNumInterfaces; interface++) {
        const struct libusb_interface_descriptor *intf_desc =
                        &config->interface[interface].altsetting[0];
        int interface_num = intf_desc->bInterfaceNumber;

        if (intf_desc->bInterfaceClass != LIBUSB_CLASS_HID
            || intf_desc->bInterfaceProtocol != 2) {
            continue;
        } else {
            for (int ep = 0; ep < intf_desc->bNumEndpoints; ep++) {
                if ((intf_desc->endpoint[ep].bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_INTERRUPT ||
                    (intf_desc->endpoint[ep].bEndpointAddress & 0x80) != LIBUSB_ENDPOINT_IN) {
                    printf("skipping interface %d, endpoint[%d] is not bulk out", interface_num, ep);
                    continue;
                } else {
                    printf("founded interface %d, endpoint[%d]\n", interface_num, ep);
                    printf("wMaxPacketSize : %d\n", intf_desc->endpoint[ep].wMaxPacketSize);
                    endpoint_in = intf_desc->endpoint[ep].bEndpointAddress;
                    interface_n = interface;
                    goto founded;
                }
            }
        }
    }

founded:
    rc = libusb_open(dev, dev_handle);
    if (rc) {
        fprintf(stderr, "failed to open device : %s\n", libusb_strerror(rc));
        exit(1);
    }

    /* detach origin driver */
    libusb_set_auto_detach_kernel_driver(*dev_handle, true);

    /* clain interface */
    rc = libusb_claim_interface(*dev_handle, interface_n);
    if (rc) {
        fprintf(stderr, "failed to claim interface\n");
        exit(1);
    }

    /* free alloced resource */
    libusb_free_config_descriptor(config);

    return dr_vidpid_ok;
}

static void sigint_handler()
{
    libusb_release_interface(dev_handle, interface_n);
    libusb_close(dev_handle);
    libusb_exit(NULL);
}

int main()
{
    int i, rc;

    /* signal handle */
    signal(SIGINT, sigint_handler);

    rc = libusb_init(NULL);
    if (rc < 0) {
        printf("failed to initialize libusb");
        exit(1);
    }

    /* get device list */
    if ((num_devices = libusb_get_device_list(NULL, &devs)) < 0) {
        printf("libusb get device list failed ! %s \n", libusb_error_name(num_devices));
        libusb_exit(NULL);
        exit(1);
    }

    /* for each device, get config descriptor */
    for (i = 0; i < num_devices; i++) {
        dev = devs[i];

        rc = usb_mouse_probe(dev, &dev_handle);
        if (rc == dr_vidpid_ok)
            break;
    }

    if (i == num_devices) {
        printf("device not founded! exiting ...\n");
        goto err_exit;
    }


    printf("device opened! ep addr : 0x%02x\n", endpoint_out);
    int size;
    uint8_t data[8];

    while (1) {
        rc = libusb_interrupt_transfer(dev_handle, endpoint_in, data, sizeof(data), &size,
                                       3000);
        if (rc) {
            printf("usb mouse read error : %s\n", libusb_strerror(rc));
            continue;
        }
        
	    if (data[0] & 0x01) {
            printf("left key pressed!\n");
	    }
	    if (data[0] & 0x02) {
            printf("right key pressed!\n");
	    }
	    if (data[0] & 0x04) {
            printf("middle key pressed!\n");
	    }
	    if (data[0] & 0x08) {
            printf("side key pressed!\n");
	    }
	    if (data[0] & 0x10) {
            printf("extra key pressed!\n");
	    }

        printf("rel x : %d\n", data[1]);
        printf("rel y : %d\n", data[2]);
        printf("rel wheel : %d\n", data[3]);
    }


err_exit:
    libusb_free_device_list(devs, true);
    libusb_release_interface(dev_handle, interface_n);
    libusb_close(dev_handle);
    libusb_exit(NULL);

    return 0;
}