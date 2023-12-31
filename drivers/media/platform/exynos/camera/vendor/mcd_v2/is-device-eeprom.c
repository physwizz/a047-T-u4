/*
 * Samsung Exynos5 SoC series IS driver
 *
 * exynos5 is core functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <exynos-is-sensor.h>
#include "is-core.h"
#include "is-device-sensor.h"
#include "is-resourcemgr.h"
#include "is-hw.h"
#include "is-device-eeprom.h"
#include "is-vender-specific.h"

#define DRIVER_NAME "is_eeprom_i2c"
#define DRIVER_NAME_REAR "rear-eeprom-i2c"
#define DRIVER_NAME_FRONT "front-eeprom-i2c"
#define DRIVER_NAME_REAR2 "rear2-eeprom-i2c"
#define DRIVER_NAME_REAR3 "rear3-eeprom-i2c"
#define DRIVER_NAME_REAR4 "rear4-eeprom-i2c"

extern const struct is_vender_rom_addr *vender_rom_addr[SENSOR_POSITION_MAX];

int is_eeprom_parse_dt(struct device_node *dnode, int rom_id)
{
	int ret = 0;
	int share_rom_position;
	
	struct is_core *core;
	struct is_vender_specific *specific;

	if (!is_dev)
		return -ENODEV;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	if (!core)
		return -ENODEV;

	specific = core->vender.private_data;

	ret = of_property_read_u32(dnode, "use_common_rom_position", &share_rom_position);
	if(!ret) {
		probe_info("use_common_rom_position %d\n", share_rom_position);
		specific->rom_share[share_rom_position].check_rom_share = true;
		specific->rom_share[share_rom_position].share_position = rom_id;
	}

	return 0;
}

int sensor_eeprom_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct is_core *core;
	static bool probe_retried = false;
	struct is_device_eeprom *device;
	struct is_vender_specific *specific;

	if (!is_dev)
		goto probe_defer;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	if (!core)
		goto probe_defer;

	specific = core->vender.private_data;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		probe_err("No I2C functionality found\n");
		return -ENODEV;
	}

	if (id->driver_data < SENSOR_POSITION_MAX) {
		specific->rom_client[id->driver_data] = client;
		specific->rom_data[id->driver_data].rom_type = ROM_TYPE_EEPROM;
		specific->rom_data[id->driver_data].rom_valid = true;
		specific->rom_bank[id->driver_data] = 0;

		specific->rom_cal_map_addr[id->driver_data] = vender_rom_addr[id->driver_data];
	} else {
		probe_err("eeprom device is failed! id=%ld\n", id->driver_data);
		return -ENODEV;
	}

	device = kzalloc(sizeof(struct is_device_eeprom), GFP_KERNEL);
	if (!device) {
		probe_err("is_device_eeprom is NULL");
		return -ENOMEM;
	}

	device->client = client;
	device->core = core;
	device->driver_data = id->driver_data;

	i2c_set_clientdata(client, device);

	if (client->dev.of_node) {
		if(is_eeprom_parse_dt(client->dev.of_node, device->driver_data)) {
			probe_err("parsing device tree is fail");
			kfree(device);
			return -ENODEV;
		}
	}

	probe_info("%s %s[%ld]: is_sensor_eeprom probed!\n",
		dev_driver_string(&client->dev), dev_name(&client->dev), id->driver_data);

	return 0;

probe_defer:
	if (probe_retried) {
		probe_err("probe has already been retried!!");
	}

	probe_retried = true;
	probe_err("core device is not yet probed");
	return -EPROBE_DEFER;

}

static int sensor_eeprom_remove(struct i2c_client *client)
{
	int ret = 0;
	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id exynos_is_sensor_eeprom_match[] = {
	{
		.compatible = "samsung,rear-eeprom-i2c", .data = (void *)SENSOR_POSITION_REAR
	},
	{
		.compatible = "samsung,front-eeprom-i2c", .data = (void *)SENSOR_POSITION_FRONT
	},
	{
		.compatible = "samsung,rear3-eeprom-i2c",
	},
	{
		.compatible = "samsung,rear4-eeprom-i2c",
	},
	{},
};
#endif

static const struct i2c_device_id sensor_eeprom_idt[] = {
	{ DRIVER_NAME_REAR, SENSOR_POSITION_REAR },
	{ DRIVER_NAME_FRONT, SENSOR_POSITION_FRONT },
	{ DRIVER_NAME_REAR3, SENSOR_POSITION_REAR3 },
	{ DRIVER_NAME_REAR4, SENSOR_POSITION_REAR4 },
	{},
};

static struct i2c_driver sensor_eeprom_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = exynos_is_sensor_eeprom_match
#endif
	},
	.probe	= sensor_eeprom_probe,
	.remove	= sensor_eeprom_remove,
	.id_table = sensor_eeprom_idt
};

static int __init sensor_eeprom_load(void)
{
        return i2c_add_driver(&sensor_eeprom_driver);
}

static void __exit sensor_eeprom_unload(void)
{
        i2c_del_driver(&sensor_eeprom_driver);
}

module_init(sensor_eeprom_load);
module_exit(sensor_eeprom_unload);

MODULE_AUTHOR("Kyoungho Yun");
MODULE_DESCRIPTION("Camera eeprom driver");
MODULE_LICENSE("GPL v2");
