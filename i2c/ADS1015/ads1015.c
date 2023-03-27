#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/capability.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

/* ADS101X operating modes */
enum {
	ADS101X_MODE_SINGLE_SHOT,
	ADS101X_MODE_CONTINUOUS_CONVERSION,
};

struct ads101x_regs {
	u8 conversion;
	u8 config;
	u8 lo_thresh;
	u8 hi_thresh;
}

struct ads101x_regs ads101x_regs_ads1015 = {
	.conversion = 0x00,
	.config     = 0x01,
	.lo_thresh  = 0x02,
	.hi_thresh  = 0x03,
}

struct ads101x_data {
	struct i2c_client *client;
	struct ads101x

	struct work_struct work;
	wait_queue_head_t wait_queue;
	spinlock_t lock;	
};

static void ads101x_work_handler(struct work_struct *work)
{
	struct ads101x_data *ads101x = container_of(work, \
						struct ads101x_data, work);
	/* do some work */
}

static int 
ads101x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ads101x_data *ads101x;

	ads101x = devm_kzalloc(&client->dev, sizeof(struct ads101x_data),
			GFP_KERNEL);
	if(!ads101x)
		return -ENOMEM;

	INIT_WORK(&ads101x->work, )
	init_waitqueue_head(&ads101x->wait_queue);
	spin_lock_init(&ads101x->lock);
	
	ads101x->client = client;
	i2c_set_clientdata(client, ads101x);

	return 0;
}

static int
ads101x_remove(struct i2c_client *client)
{
	struct ads101x_data *ads101x = i2c_get_clientdata(client);

	kfree(ads101x);
	return 0;
}

static struct of_device_id ads101x_of_match_table[] = {
	{ .compatible = "ti,ads101x" },
	{},
};
MODULE_DEVICE_TABLE(of, ads101x_of_match_table);

static struct i2c_device_id ads101x_id_table[] = {
	{ "ads101x", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, ads101x_id_table);

static struct i2c_driver ads101x_i2c_driver = {
	.probe = ads101x_probe,
	.remove = ads101x_remove,
	.driver = {
		.name = "ads101x",
		.of_match_table = of_match_ptr(ads101x_of_match_table),
	},

	.id_table = ads101x_id_table,
};
module_i2c_driver(ads101x_i2c_driver);

MODULE_AUTHOR("IotaHydrae writeforever@foxmail.com");
MODULE_DESCRIPTION("a i2c driver for ads101x");
MODULE_LICENSE("GPL");
