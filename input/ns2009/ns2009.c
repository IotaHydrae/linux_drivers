//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NS2009 touchscreen driver
 *
 * Copyright (C)
 *
 * Based on
 */
 
#include <linux/input.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

static int ns2009_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	return 0;
}

static int ns2009_remove(struct i2c_client *client)
{
	return 0;
}


static const struct of_device_id ns2009_of_match[] = {
	{ .compatible = "ns2009" },
	{ }
};

static struct i2c_driver ns2009_driver = {
	.driver = {
		.name = "ns2009",
		.of_match_table = of_match_ptr(ns2009_of_match),
		/* .pm   = *ns2009_pm_ops, */
	},

	//.id_table     = ns2009_idtable,
	.probe        = ns2009_probe,
	.remove       = ns2009_remove,

	/* if device autodetection is needed: */
	/*
	 * .class	    = I2C_CLASS_SOMETHING,
	 * .detect	    = ns2009_detect,
	 * .address_list    = normal_i2c,
	 *
	 * .shutdown	    = ns2009_shutdown, //optional
	 * .command	    = ns2009_command,  //optional, deprecated 
	 * */
};

/*static int __init ns2009_init(void)
{
	return i2c_add_driver(&ns2009_driver);
}
module_init(ns2009_init);

static void __exit ns2009_exit(void)
{
	return i2c_del_driver(&ns2009_driver);
}
module_exit(ns2009_exit);
*/
module_i2c_driver(ns2009_driver);

MODULE_AUTHOR("IotaHydrae <writeforever@foxmail.com>");
MODULE_ALIAS("ns2009-i2c");
MODULE_DESCRIPTION("Device driver or NS2009 Touchscreen Driver IC");
MODULE_LICENSE("GPL");
