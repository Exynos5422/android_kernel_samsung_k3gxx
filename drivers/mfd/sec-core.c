/*
 * sec-core.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd
 *              http://www.samsung.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
<<<<<<< HEAD
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/irqnr.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/mutex.h>
#include <linux/rtc.h>
=======
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/mutex.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <linux/mfd/core.h>
#include <linux/mfd/samsung/core.h>
#include <linux/mfd/samsung/irq.h>
#include <linux/mfd/samsung/rtc.h>
#include <linux/regmap.h>

static struct mfd_cell s5m8751_devs[] = {
	{
		.name = "s5m8751-pmic",
	}, {
		.name = "s5m-charger",
	}, {
		.name = "s5m8751-codec",
	},
};

static struct mfd_cell s5m8763_devs[] = {
	{
		.name = "s5m8763-pmic",
	}, {
		.name = "s5m-rtc",
	}, {
		.name = "s5m-charger",
	},
};

static struct mfd_cell s5m8767_devs[] = {
	{
		.name = "s5m8767-pmic",
	}, {
		.name = "s5m-rtc",
	},
};

static struct mfd_cell s2mps11_devs[] = {
	{
		.name = "s2mps11-pmic",
<<<<<<< HEAD
	}, {
		.name = "s2m-rtc",
	},
};

static struct mfd_cell s2mps13_devs[] = {
	{
		.name = "s2mps13-pmic",
	}, {
		.name = "s2m-rtc",
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	},
};

#ifdef CONFIG_OF
static struct of_device_id sec_dt_match[] = {
	{	.compatible = "samsung,s5m8767-pmic",
		.data = (void *)S5M8767X,
	},
<<<<<<< HEAD
	{	.compatible = "samsung,s2mps13-pmic",
		.data = (void *)S2MPS13X,
	},
	{	.compatible = "samsung,s2mps11-pmic",
		.data = (void *)S2MPS11X,
	},
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	{},
};
#endif

<<<<<<< HEAD
int sec_reg_read(struct sec_pmic_dev *sec_pmic, u32 reg, void *dest)
=======
int sec_reg_read(struct sec_pmic_dev *sec_pmic, u8 reg, void *dest)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return regmap_read(sec_pmic->regmap, reg, dest);
}
EXPORT_SYMBOL_GPL(sec_reg_read);

<<<<<<< HEAD
int sec_bulk_read(struct sec_pmic_dev *sec_pmic, u32 reg, int count, u8 *buf)
=======
int sec_bulk_read(struct sec_pmic_dev *sec_pmic, u8 reg, int count, u8 *buf)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return regmap_bulk_read(sec_pmic->regmap, reg, buf, count);
}
EXPORT_SYMBOL_GPL(sec_bulk_read);

<<<<<<< HEAD
int sec_reg_write(struct sec_pmic_dev *sec_pmic, u32 reg, u32 value)
=======
int sec_reg_write(struct sec_pmic_dev *sec_pmic, u8 reg, u8 value)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return regmap_write(sec_pmic->regmap, reg, value);
}
EXPORT_SYMBOL_GPL(sec_reg_write);

<<<<<<< HEAD
int sec_bulk_write(struct sec_pmic_dev *sec_pmic, u32 reg, int count, u8 *buf)
=======
int sec_bulk_write(struct sec_pmic_dev *sec_pmic, u8 reg, int count, u8 *buf)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return regmap_raw_write(sec_pmic->regmap, reg, buf, count);
}
EXPORT_SYMBOL_GPL(sec_bulk_write);

<<<<<<< HEAD
int sec_reg_update(struct sec_pmic_dev *sec_pmic, u32 reg, u32 val, u32 mask)
=======
int sec_reg_update(struct sec_pmic_dev *sec_pmic, u8 reg, u8 val, u8 mask)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return regmap_update_bits(sec_pmic->regmap, reg, mask, val);
}
EXPORT_SYMBOL_GPL(sec_reg_update);

<<<<<<< HEAD
int sec_rtc_read(struct sec_pmic_dev *sec_pmic, u32 reg, void *dest)
{
	return regmap_read(sec_pmic->rtc_regmap, reg, dest);
}
EXPORT_SYMBOL_GPL(sec_rtc_read);

int sec_rtc_bulk_read(struct sec_pmic_dev *sec_pmic, u32 reg, int count,
		u8 *buf)
{
	return regmap_bulk_read(sec_pmic->rtc_regmap, reg, buf, count);
}
EXPORT_SYMBOL_GPL(sec_rtc_bulk_read);

int sec_rtc_write(struct sec_pmic_dev *sec_pmic, u32 reg, u32 value)
{
	return regmap_write(sec_pmic->rtc_regmap, reg, value);
}
EXPORT_SYMBOL_GPL(sec_rtc_write);

int sec_rtc_bulk_write(struct sec_pmic_dev *sec_pmic, u32 reg, int count,
		u8 *buf)
{
	return regmap_raw_write(sec_pmic->rtc_regmap, reg, buf, count);
}
EXPORT_SYMBOL_GPL(sec_rtc_bulk_write);

int sec_rtc_update(struct sec_pmic_dev *sec_pmic, u32 reg, u32 val,
		u32 mask)
{
	return regmap_update_bits(sec_pmic->rtc_regmap, reg, mask, val);
}
EXPORT_SYMBOL_GPL(sec_rtc_update);

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static struct regmap_config sec_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};


#ifdef CONFIG_OF
<<<<<<< HEAD
static struct sec_platform_data *sec_pmic_i2c_parse_dt_pdata(
					struct device *dev)
{
	struct sec_platform_data *pdata;
	struct device_node *np = dev->of_node;
	int ret;
	u32 val;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "failed to allocate platform data\n");
		return ERR_PTR(-ENOMEM);
	}
	dev->platform_data = pdata;
	pdata->irq_base = -1;

	/* WTSR, SMPL */
	pdata->wtsr_smpl = devm_kzalloc(dev, sizeof(*pdata->wtsr_smpl),
			GFP_KERNEL);
	if (!pdata->wtsr_smpl)
		return ERR_PTR(-ENOMEM);

	ret = of_property_read_u32(np, "wtsr_en", &val);
	if (ret)
		return ERR_PTR(ret);
	pdata->wtsr_smpl->wtsr_en = !!val;

	ret = of_property_read_u32(np, "smpl_en", &val);
	if (ret)
		return ERR_PTR(ret);
	pdata->wtsr_smpl->smpl_en = !!val;

	ret = of_property_read_u32(np, "wtsr_timer_val",
			&pdata->wtsr_smpl->wtsr_timer_val);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "smpl_timer_val",
			&pdata->wtsr_smpl->smpl_timer_val);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "check_jigon", &val);
	if (ret)
		return ERR_PTR(ret);
	pdata->wtsr_smpl->check_jigon = !!val;

	/* init time */
	pdata->init_time = devm_kzalloc(dev, sizeof(*pdata->init_time),
			GFP_KERNEL);
	if (!pdata->init_time)
		return ERR_PTR(-ENOMEM);

	ret = of_property_read_u32(np, "init_time,sec",
			&pdata->init_time->tm_sec);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,min",
			&pdata->init_time->tm_min);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,hour",
			&pdata->init_time->tm_hour);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,mday",
			&pdata->init_time->tm_mday);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,mon",
			&pdata->init_time->tm_mon);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,year",
			&pdata->init_time->tm_year);
	if (ret)
		return ERR_PTR(ret);

	ret = of_property_read_u32(np, "init_time,wday",
			&pdata->init_time->tm_wday);
	if (ret)
		return ERR_PTR(ret);

	return pdata;
=======
/*
 * Only the common platform data elements for s5m8767 are parsed here from the
 * device tree. Other sub-modules of s5m8767 such as pmic, rtc , charger and
 * others have to parse their own platform data elements from device tree.
 *
 * The s5m8767 platform data structure is instantiated here and the drivers for
 * the sub-modules need not instantiate another instance while parsing their
 * platform data.
 */
static struct sec_platform_data *sec_pmic_i2c_parse_dt_pdata(
					struct device *dev)
{
	struct sec_platform_data *pd;

	pd = devm_kzalloc(dev, sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		dev_err(dev, "could not allocate memory for pdata\n");
		return ERR_PTR(-ENOMEM);
	}

	/*
	 * ToDo: the 'wakeup' member in the platform data is more of a linux
	 * specfic information. Hence, there is no binding for that yet and
	 * not parsed here.
	 */

	return pd;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}
#else
static struct sec_platform_data *sec_pmic_i2c_parse_dt_pdata(
					struct device *dev)
{
<<<<<<< HEAD
	return NULL;
=======
	return 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}
#endif

static inline int sec_i2c_get_driver_data(struct i2c_client *i2c,
						const struct i2c_device_id *id)
{
#ifdef CONFIG_OF
	if (i2c->dev.of_node) {
		const struct of_device_id *match;
		match = of_match_node(sec_dt_match, i2c->dev.of_node);
		return (int)match->data;
	}
#endif
	return (int)id->driver_data;
}

<<<<<<< HEAD
static struct sec_pmic_dev *g_sec_pmic;

int sec_pmic_get_irq_base(void)
{
	int irq_base;

	if (!g_sec_pmic) {
		pr_err("%s: Failed to get irq base: sec_pmic is null\n", __func__);
		return -ENODEV;
	}

	irq_base = regmap_irq_chip_get_base(g_sec_pmic->irq_data);
	if (!irq_base) {
		pr_err("%s: Failed to get irq base %d\n", __func__, irq_base);
		return -ENODEV;
	}

	return irq_base;
}

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static int sec_pmic_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct sec_platform_data *pdata = i2c->dev.platform_data;
	struct sec_pmic_dev *sec_pmic;
	int ret;

	sec_pmic = devm_kzalloc(&i2c->dev, sizeof(struct sec_pmic_dev),
				GFP_KERNEL);
	if (sec_pmic == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, sec_pmic);
	sec_pmic->dev = &i2c->dev;
	sec_pmic->i2c = i2c;
	sec_pmic->irq = i2c->irq;
	sec_pmic->type = sec_i2c_get_driver_data(i2c, id);

	if (sec_pmic->dev->of_node) {
		pdata = sec_pmic_i2c_parse_dt_pdata(sec_pmic->dev);
		if (IS_ERR(pdata)) {
			ret = PTR_ERR(pdata);
			return ret;
		}
		pdata->device_type = sec_pmic->type;
	}
<<<<<<< HEAD

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (pdata) {
		sec_pmic->device_type = pdata->device_type;
		sec_pmic->ono = pdata->ono;
		sec_pmic->irq_base = pdata->irq_base;
<<<<<<< HEAD
		sec_pmic->wakeup = true;
		sec_pmic->pdata = pdata;
		sec_pmic->irq = i2c->irq;
=======
		sec_pmic->wakeup = pdata->wakeup;
		sec_pmic->pdata = pdata;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	sec_pmic->regmap = devm_regmap_init_i2c(i2c, &sec_regmap_config);
	if (IS_ERR(sec_pmic->regmap)) {
		ret = PTR_ERR(sec_pmic->regmap);
		dev_err(&i2c->dev, "Failed to allocate register map: %d\n",
			ret);
		return ret;
	}

	sec_pmic->rtc = i2c_new_dummy(i2c->adapter, RTC_I2C_ADDR);
<<<<<<< HEAD
	i2c_set_clientdata(sec_pmic->rtc, sec_pmic);
	sec_pmic->rtc_regmap = devm_regmap_init_i2c(sec_pmic->rtc, &sec_regmap_config);
	if (IS_ERR(sec_pmic->rtc_regmap)) {
		ret = PTR_ERR(sec_pmic->rtc_regmap);
		dev_err(&sec_pmic->rtc->dev,"Failed to allocate register map: %d\n",
			ret);
		return ret;
	}
=======
	if (!sec_pmic->rtc) {
		dev_err(&i2c->dev, "Failed to allocate I2C for RTC\n");
		return -ENODEV;
	}
	i2c_set_clientdata(sec_pmic->rtc, sec_pmic);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (pdata && pdata->cfg_pmic_irq)
		pdata->cfg_pmic_irq();

	sec_irq_init(sec_pmic);

	pm_runtime_set_active(sec_pmic->dev);

	switch (sec_pmic->device_type) {
	case S5M8751X:
		ret = mfd_add_devices(sec_pmic->dev, -1, s5m8751_devs,
				      ARRAY_SIZE(s5m8751_devs), NULL, 0, NULL);
		break;
	case S5M8763X:
		ret = mfd_add_devices(sec_pmic->dev, -1, s5m8763_devs,
				      ARRAY_SIZE(s5m8763_devs), NULL, 0, NULL);
		break;
	case S5M8767X:
		ret = mfd_add_devices(sec_pmic->dev, -1, s5m8767_devs,
				      ARRAY_SIZE(s5m8767_devs), NULL, 0, NULL);
		break;
	case S2MPS11X:
		ret = mfd_add_devices(sec_pmic->dev, -1, s2mps11_devs,
				      ARRAY_SIZE(s2mps11_devs), NULL, 0, NULL);
		break;
<<<<<<< HEAD
	case S2MPS13X:
		ret = mfd_add_devices(sec_pmic->dev, -1, s2mps13_devs,
				      ARRAY_SIZE(s2mps13_devs), NULL, 0, NULL);
		break;
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	default:
		/* If this happens the probe function is problem */
		BUG();
	}

	if (ret < 0)
		goto err;

<<<<<<< HEAD
	g_sec_pmic = sec_pmic;

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return ret;

err:
	mfd_remove_devices(sec_pmic->dev);
	sec_irq_exit(sec_pmic);
	i2c_unregister_device(sec_pmic->rtc);
	return ret;
}

static int sec_pmic_remove(struct i2c_client *i2c)
{
	struct sec_pmic_dev *sec_pmic = i2c_get_clientdata(i2c);

	mfd_remove_devices(sec_pmic->dev);
	sec_irq_exit(sec_pmic);
<<<<<<< HEAD
	regmap_exit(sec_pmic->rtc_regmap);
	regmap_exit(sec_pmic->regmap);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	i2c_unregister_device(sec_pmic->rtc);
	return 0;
}

static const struct i2c_device_id sec_pmic_id[] = {
	{ "sec_pmic", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sec_pmic_id);

<<<<<<< HEAD
#ifdef CONFIG_PM
static int sec_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct sec_pmic_dev *sec_pmic = i2c_get_clientdata(i2c);

	if (sec_pmic->wakeup)
		enable_irq_wake(sec_pmic->irq);

	disable_irq(sec_pmic->irq);
	return 0;
}

static int sec_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct sec_pmic_dev *sec_pmic = i2c_get_clientdata(i2c);

	if (sec_pmic->wakeup)
		disable_irq_wake(sec_pmic->irq);

	enable_irq(sec_pmic->irq);
	return 0;
}
#else
#define sec_suspend	NULL
#define sec_resume	NULL
#endif /* CONFIG_PM */

const struct dev_pm_ops sec_pmic_apm = {
	.suspend = sec_suspend,
	.resume = sec_resume,
};

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static struct i2c_driver sec_pmic_driver = {
	.driver = {
		   .name = "sec_pmic",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(sec_dt_match),
<<<<<<< HEAD
		   .pm = &sec_pmic_apm,		   
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	},
	.probe = sec_pmic_probe,
	.remove = sec_pmic_remove,
	.id_table = sec_pmic_id,
};

static int __init sec_pmic_init(void)
{
	return i2c_add_driver(&sec_pmic_driver);
}

subsys_initcall(sec_pmic_init);

static void __exit sec_pmic_exit(void)
{
	i2c_del_driver(&sec_pmic_driver);
}
module_exit(sec_pmic_exit);

MODULE_AUTHOR("Sangbeom Kim <sbkim73@samsung.com>");
<<<<<<< HEAD
MODULE_DESCRIPTION("Core support for the SAMSUNG MFD");
=======
MODULE_DESCRIPTION("Core support for the S5M MFD");
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
MODULE_LICENSE("GPL");
