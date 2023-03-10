/**
 * @file ssd1681.c
 * @author Iota Hydrae (writeforever@foxmail.com)
 * @brief linux device driver for ssd1681 e-paper driver IC
 * @version 0.1
 * @date 2023-03-08
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/stdarg.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>

#include <linux/wait.h>
#include <linux/spinlock.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/regmap.h>

#include <linux/uaccess.h>

#include <video/mipi_display.h>

#define DRV_NAME "ssd1681"

#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

enum ssd1681_refr_mode {
    SSD1681_REFR_MODE_PART = 0x00,
    SSD1681_REFR_MODE_FULL = 0x01,
};

enum ssd1681_command {
    DUMMY = 0x00,
};

#define SPI_BUF_LEN     256
#define PANEL_BUSY_TIMEOUT_MS   200

struct ssd1681_dev {
        /* Applicable to similar devices */
        struct device           *dev;
        struct spi_device       *spi;
        const s16               *init_sequence;
        u8                      *buf;
        struct {
                void *buf;
                size_t len;
        } txbuf;
        struct {
                struct gpio_desc *reset;
                struct gpio_desc *dc;
                struct gpio_desc *cs;
                struct gpio_desc *busy;
        } gpio;
        struct completion       complete;

        /* device specific */
        u32                     xres;
        u32                     yres;
        u32                     refr_mode;
        u32                     wait;
        u32                     busy;
};

static const s16 default_init_sequence[] = {
    -1, 0x12,   /* software reset */
    
    4, 0x01, 0xc7, 0x00, 0x01,
    
    2, 0x11, 0x01,
    
    2, 0x44, 0x00, 0x18,
    
    4, 0x45, 0xc7, 0x00, 0x00, 0x00,
    
    2, 0x3c, 0x05,
    
    2, 0x18, 0x80,

    -5, 0x4e, 0x00, 0x4f, 0xc7, 0x00,

    /* set default refr mode here */
};

static void ssd1681_write_reg(struct ssd1681_dev *sd, int len, ...)
{
        va_list args;
        int i;

        u8 *buf = sd->txbuf.buf;

        va_start(args, len);
        for (i =0; i < len; i++) {
            buf[i] = va_arg(args, unsigned int);
            pr_debug("va arg : 0x%02x\n", buf[i]);
        }
        va_end(args);

        spi_write(sd->spi, buf, len);
}
#define write_reg(dev, ...) \
        ssd1681_write_reg(dev, NUMARGS(__VA_ARGS__), __VA_ARGS__)

static void ssd1681_set_addr_win(struct ssd1681_dev *sd, int xs, int ys, int xe,
                                 int ye)
{
	write_reg(sd, MIPI_DCS_SET_COLUMN_ADDRESS,
		  (xs >> 8) & 0xFF, xs & 0xFF, (xe >> 8) & 0xFF, xe & 0xFF);

	write_reg(sd, MIPI_DCS_SET_PAGE_ADDRESS,
		  (ys >> 8) & 0xFF, ys & 0xFF, (ye >> 8) & 0xFF, ye & 0xFF);

	write_reg(sd, MIPI_DCS_WRITE_MEMORY_START);
}

static int ssd1681_init_display(struct ssd1681_dev *sd)
{
        int step;
        const s16 *index;
        
        if (!sd->init_sequence)
                return -EINVAL;

        index = sd->init_sequence;
        for (;;) {
                if (!index)
                        break;
                /* need for waiting? */
                step = (unsigned int)*index;
                memcpy(sd->buf, (index + 1), step);
                // spi_write(sd->spi, sd->buf, step);
                if (*index > 0) {
                        pr_debug("waiting for busy ...\n");
                } else {
                        pr_debug("relax, here nothing to do ...\n");
                }
                
                index += (step + 1);
        }

        return 0;
}

static int ssd1681_hw_init(struct ssd1681_dev *sd)
{
        pr_debug("%s, initializing hardware ...\n", __func__);
        // ssd1681_init_display(sd);
        ssd1681_set_addr_win(sd, 0, 0, 200, 200);
        return 0;
}

static int ssd1681_request_one_gpio(struct ssd1681_dev *sd,
                                    const char *name, int index,
                                    struct gpio_desc **gpiop)
{
        struct device *dev = sd->dev;
        struct device_node *np = dev->of_node;
        int gpio, flags, rc = 0;
        enum of_gpio_flags of_flags;

        if (of_find_property(np, name, NULL)) {
                gpio = of_get_named_gpio_flags(np, name, index, &of_flags);
                if (gpio == -ENOENT)
                        return 0;
                if (gpio == -EPROBE_DEFER)
                        return gpio;
                if (gpio < 0) {
                        dev_err(dev,
                                "failed to get '%s' from DT\n", name);
                        return gpio;
                }

                flags = (of_flags & OF_GPIO_ACTIVE_LOW) ? GPIOF_OUT_INIT_LOW :
                        GPIOF_OUT_INIT_HIGH;
                rc = devm_gpio_request_one(dev, gpio, flags,
                                           dev->driver->name);
                if (rc) {
                        dev_err(dev,
                                "gpio_request_one('%s'=%d) failed with %d\n",
                                name, gpio, rc);
                        return rc;
                }
                if (gpiop)
                        *gpiop = gpio_to_desc(gpio);
                pr_debug("%s : '%s' = GPIO%d\n",
                        __func__, name, gpio);
        }

        return rc;
}

static int ssd1681_of_config(struct ssd1681_dev *sd)
{
        int rc;
        pr_debug("%s, configure from dt\n", __func__);

        rc = ssd1681_request_one_gpio(sd, "reset-gpios", 0, &sd->gpio.reset);
        if (rc)
                return rc;
        rc = ssd1681_request_one_gpio(sd, "dc-gpios", 0, &sd->gpio.dc);
        if (rc)
                return rc;
        rc = ssd1681_request_one_gpio(sd, "busy-gpios", 0, &sd->gpio.busy);
        if (rc)
                return rc;
        rc = ssd1681_request_one_gpio(sd, "cs-gpios", 0, &sd->gpio.cs);
        if (rc)
                return rc;

        return 0;
}

static int ssd1681_probe(struct spi_device *spi)
{
        struct ssd1681_dev *sd;
        struct device *dev = &spi->dev;

        /* memory resource alloc */
        sd = kmalloc(sizeof(struct ssd1681_dev), GFP_KERNEL);
        if (!sd) {
                dev_err(dev, "failed to alloc sd memory!\n");
                return -ENOMEM;
        }

        sd->buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
        if (!sd->buf) {
                dev_err(dev, "failed to alloc buf memory!\n");
                return -ENOMEM;
        }

        sd->txbuf.buf = kmalloc(SPI_BUF_LEN, GFP_KERNEL);
        if (!sd->txbuf.buf) {
                dev_err(dev, "failed to alloc txbuf!\n");
                return -ENOMEM;
        }

        sd->spi = spi;
        sd->dev = dev;
        sd->init_sequence = default_init_sequence;
        spi_set_drvdata(spi, sd);

        init_completion(&sd->complete);

        ssd1681_of_config(sd);
        ssd1681_hw_init(sd);

        return 0;
}

static int ssd1681_remove(struct spi_device *spi)
{
        struct ssd1681_dev *sd = spi_get_drvdata(spi);

        kfree(sd->buf);
        kfree(sd->txbuf.buf);

        kfree(sd);
        return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ssd1681_dt_ids[] = {
        { .compatible = "solomon,ssd1681" },
        { /* KEEP THIS */ },
};
#endif

static const struct spi_device_id ssd1681_spi_ids[] = {
        { "ssd1681" },
        { /* KEEP THIS */ },
};

static struct spi_driver ssd1681_drv = {
        .driver = {
                .name = DRV_NAME,
                .of_match_table = of_match_ptr(ssd1681_dt_ids),
        },
        .id_table = ssd1681_spi_ids,
        .probe = ssd1681_probe,
        .remove = ssd1681_remove,
};

module_spi_driver(ssd1681_drv);

MODULE_AUTHOR("Iota Hydrae <writeforever@foxmail.com>");
MODULE_DESCRIPTION("E-Paper display driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("spi:ssd1681");