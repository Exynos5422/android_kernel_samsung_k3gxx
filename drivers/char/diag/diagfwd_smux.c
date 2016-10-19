<<<<<<< HEAD
/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2012,2014 The Linux Foundation. All rights reserved.
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/termios.h>
#include <linux/slab.h>
#include <linux/diagchar.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
<<<<<<< HEAD
#include <mach/usbdiag.h>
#include "diagchar.h"
#include "diagfwd.h"
#include "diagfwd_smux.h"
#include "diagfwd_hsic.h"
#include "diagfwd_bridge.h"

void diag_smux_event(void *priv, int event_type, const void *metadata)
{
	unsigned char *rx_buf;
	int len;

	switch (event_type) {
	case SMUX_CONNECTED:
		pr_info("diag: SMUX_CONNECTED received\n");
		driver->smux_connected = 1;
		driver->in_busy_smux = 0;
		/* read data from USB MDM channel & Initiate first write */
		queue_work(diag_bridge[SMUX].wq,
			   &diag_bridge[SMUX].diag_read_work);
		break;
	case SMUX_DISCONNECTED:
		driver->smux_connected = 0;
		driver->lcid = LCID_INVALID;
		msm_smux_close(LCID_VALID);
		pr_info("diag: SMUX_DISCONNECTED received\n");
		break;
	case SMUX_WRITE_DONE:
		pr_debug("diag: SMUX Write done\n");
		break;
	case SMUX_WRITE_FAIL:
		pr_info("diag: SMUX Write Failed\n");
		break;
	case SMUX_READ_FAIL:
		pr_info("diag: SMUX Read Failed\n");
		break;
	case SMUX_READ_DONE:
		len = ((struct smux_meta_read *)metadata)->len;
		rx_buf = ((struct smux_meta_read *)metadata)->buffer;
		driver->write_ptr_mdm->length = len;
		diag_device_write(driver->buf_in_smux, SMUX_DATA,
						 driver->write_ptr_mdm);
=======
#include <linux/usb/usbdiag.h>

#include "diagchar.h"
#include "diagfwd_bridge.h"
#include "diagfwd_smux.h"

struct diag_smux_info diag_smux[NUM_SMUX_DEV] = {
	{
		.id = SMUX_1,
		.lcid = SMUX_USB_DIAG_0,
		.dev_id = DIAGFWD_SMUX,
		.name = "SMUX_1",
		.read_buf = NULL,
		.read_len = 0,
		.in_busy = 0,
		.enabled = 0,
		.opened = 0,
	},
};

static void diag_smux_event(void *priv, int event_type, const void *metadata)
{
	int len = 0;
	int id = (int)priv;
	unsigned char *rx_buf = NULL;
	struct diag_smux_info *ch = NULL;

	if (id < 0 || id >= NUM_SMUX_DEV)
		return;

	ch = &diag_smux[id];
	if (metadata) {
		len = ((struct smux_meta_read *)metadata)->len;
		rx_buf = ((struct smux_meta_read *)metadata)->buffer;
	}

	switch (event_type) {
	case SMUX_CONNECTED:
		pr_info("diag: SMUX_CONNECTED received, ch: %d\n", ch->id);
		ch->opened = 1;
		ch->in_busy = 0;
		break;
	case SMUX_DISCONNECTED:
		ch->opened = 0;
		msm_smux_close(ch->lcid);
		pr_info("diag: SMUX_DISCONNECTED received, ch: %d\n", ch->id);
		break;
	case SMUX_WRITE_DONE:
		pr_debug("diag: SMUX Write done, ch: %d\n", ch->id);
		diag_remote_dev_write_done(ch->dev_id, rx_buf, len, ch->id);
		break;
	case SMUX_WRITE_FAIL:
		pr_info("diag: SMUX Write Failed, ch: %d\n", ch->id);
		break;
	case SMUX_READ_FAIL:
		pr_info("diag: SMUX Read Failed, ch: %d\n", ch->id);
		break;
	case SMUX_READ_DONE:
		ch->read_buf = rx_buf;
		ch->read_len = len;
		ch->in_busy = 1;
		diag_remote_dev_read_done(ch->dev_id, ch->read_buf,
					  ch->read_len);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		break;
	};
}

<<<<<<< HEAD
int diagfwd_write_complete_smux(void)
{
	pr_debug("diag: clear in_busy_smux\n");
	driver->in_busy_smux = 0;
	return 0;
}

int diagfwd_read_complete_smux(void)
{
	queue_work(diag_bridge[SMUX].wq, &diag_bridge[SMUX].diag_read_work);
	return 0;
}

int diag_get_rx_buffer(void *priv, void **pkt_priv, void **buffer, int size)
{
	if (!driver->in_busy_smux) {
		*pkt_priv = (void *)0x1234;
		*buffer = driver->buf_in_smux;
		pr_debug("diag: set in_busy_smux as 1\n");
		driver->in_busy_smux = 1;
	} else {
		pr_debug("diag: read buffer for SMUX is BUSY\n");
		return -EAGAIN;
	}
	return 0;
}

void diag_usb_read_complete_smux_fn(struct work_struct *w)
{
	diagfwd_read_complete_bridge(diag_bridge[SMUX].usb_read_ptr);
}

void diag_read_usb_smux_work_fn(struct work_struct *work)
{
	int ret;

	if (driver->diag_smux_enabled) {
		if (driver->lcid && diag_bridge[SMUX].usb_buf_out &&
			(diag_bridge[SMUX].read_len > 0) &&
				driver->smux_connected) {
			ret = msm_smux_write(driver->lcid, NULL,
			      diag_bridge[SMUX].usb_buf_out,
				 diag_bridge[SMUX].read_len);
			if (ret)
				pr_err("diag: writing to SMUX ch, r = %d, lcid = %d\n",
						 ret, driver->lcid);
		}
		diag_bridge[SMUX].usb_read_ptr->buf =
					 diag_bridge[SMUX].usb_buf_out;
		diag_bridge[SMUX].usb_read_ptr->length = USB_MAX_OUT_BUF;
		diag_bridge[SMUX].usb_read_ptr->context = (void *)SMUX;
		usb_diag_read(diag_bridge[SMUX].ch,
					 diag_bridge[SMUX].usb_read_ptr);
		return;
	}
=======
static int diag_smux_init_ch(struct diag_smux_info *ch)
{
	if (!ch)
		return -EINVAL;

	if (!ch->enabled) {
		pr_debug("diag: SMUX channel is not enabled id: %d\n", ch->id);
		return -ENODEV;
	}

	if (ch->inited) {
		pr_debug("diag: SMUX channel %d is already initialize\n",
			 ch->id);
		return 0;
	}

	ch->read_buf = kzalloc(IN_BUF_SIZE, GFP_KERNEL);
	if (!ch->read_buf)
		return -ENOMEM;

	ch->inited = 1;

	return 0;
}

static int smux_get_rx_buffer(void *priv, void **pkt_priv, void **buf,
			      int size)
{
	int id = (int)priv;
	struct diag_smux_info *ch = NULL;

	if (id < 0 || id >= NUM_SMUX_DEV)
		return -EINVAL;

	ch = &diag_smux[id];

	if (ch->in_busy) {
		pr_debug("diag: read buffer for SMUX is BUSY\n");
		return -EAGAIN;
	}

	*pkt_priv = (void *)0x1234;
	*buf = ch->read_buf;
	ch->in_busy = 1;
	return 0;
}

static int smux_open(int id)
{
	int err = 0;
	struct diag_smux_info *ch = NULL;

	if (id < 0 || id >= NUM_SMUX_DEV)
		return -EINVAL;

	ch = &diag_smux[id];
	if (ch->opened) {
		pr_debug("diag: SMUX channel %d is already connected\n",
			 ch->id);
		return 0;
	}

	err = diag_smux_init_ch(ch);
	if (err) {
		pr_err("diag: Unable to initialize SMUX channel %d, err: %d\n",
		       ch->id, err);
		return err;
	}

	err = msm_smux_open(ch->lcid, (void *)ch->id, diag_smux_event,
			    smux_get_rx_buffer);
	if (err) {
		pr_err("diag: failed to open SMUX ch %d, err: %d\n",
		       ch->id, err);
		return err;
	}
	msm_smux_tiocm_set(ch->lcid, TIOCM_DTR, 0);
	ch->opened = 1;
	pr_info("diag: SMUX ch %d is connected\n", ch->id);
	return 0;
}

static int smux_close(int id)
{
	struct diag_smux_info *ch = NULL;

	if (id < 0 || id >= NUM_SMUX_DEV)
		return -EINVAL;

	ch = &diag_smux[id];
	if (!ch->enabled) {
		pr_debug("diag: SMUX channel is not enabled id: %d\n", ch->id);
		return -ENODEV;
	}

	msm_smux_close(ch->lcid);
	ch->opened = 0;
	ch->in_busy = 1;
	kfree(ch->read_buf);
	ch->read_buf = NULL;
	return 0;
}

static int smux_queue_read(int id)
{
	return 0;
}

static int smux_write(int id, unsigned char *buf, int len, int ctxt)
{
	struct diag_smux_info *ch = NULL;

	if (id < 0 || id >= NUM_SMUX_DEV)
		return -EINVAL;

	ch = &diag_smux[id];
	return  msm_smux_write(ch->lcid, NULL, buf, len);
}

static int smux_fwd_complete(int id, unsigned char *buf, int len, int ctxt)
{
	if (id < 0 || id >= NUM_SMUX_DEV)
		return -EINVAL;

	diag_smux[id].in_busy = 0;
	return 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static int diagfwd_smux_runtime_suspend(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: suspending...\n");
	return 0;
}

static int diagfwd_smux_runtime_resume(struct device *dev)
{
	dev_dbg(dev, "pm_runtime: resuming...\n");
	return 0;
}

static const struct dev_pm_ops diagfwd_smux_dev_pm_ops = {
	.runtime_suspend = diagfwd_smux_runtime_suspend,
	.runtime_resume = diagfwd_smux_runtime_resume,
};

<<<<<<< HEAD
int diagfwd_connect_smux(void)
{
	void *priv = NULL;
	int ret = 0;

	if (driver->lcid == LCID_INVALID) {
		ret = msm_smux_open(LCID_VALID, priv, diag_smux_event,
						 diag_get_rx_buffer);
		if (!ret) {
			driver->lcid = LCID_VALID;
			msm_smux_tiocm_set(driver->lcid, TIOCM_DTR, 0);
			pr_info("diag: open SMUX ch, r = %d\n", ret);
		} else {
			pr_err("diag: failed to open SMUX ch, r = %d\n", ret);
			return ret;
		}
	}
	/* Poll USB channel to check for data*/
	queue_work(diag_bridge[SMUX].wq, &(diag_bridge[SMUX].diag_read_work));
	return ret;
}

static int diagfwd_smux_probe(struct platform_device *pdev)
{
	int ret = 0;

	pr_info("diag: SMUX probe called\n");
	driver->lcid = LCID_INVALID;
	driver->diag_smux_enabled = 1;
	if (driver->buf_in_smux == NULL) {
		driver->buf_in_smux = kzalloc(IN_BUF_SIZE, GFP_KERNEL);
		if (driver->buf_in_smux == NULL)
			goto err;
	}
	/* Only required for Local loopback test
	 * ret = msm_smux_set_ch_option(LCID_VALID,
				 SMUX_CH_OPTION_LOCAL_LOOPBACK, 0);
	 * if (ret)
	 *	pr_err("diag: error setting SMUX ch option, r = %d\n", ret);
	 */
	if (driver->write_ptr_mdm == NULL)
		driver->write_ptr_mdm = kzalloc(sizeof(struct diag_request),
								 GFP_KERNEL);
	if (driver->write_ptr_mdm == NULL)
		goto err;
	ret = diagfwd_connect_smux();
	return ret;

err:
	pr_err("diag: Could not initialize SMUX buffer\n");
	kfree(driver->buf_in_smux);
	return ret;
=======
static int diagfwd_smux_probe(struct platform_device *pdev)
{
	if (!pdev)
		return -EINVAL;

	pr_debug("diag: SMUX probe called, pdev->id: %d\n", pdev->id);
	if (pdev->id < 0 || pdev->id >= NUM_SMUX_DEV) {
		pr_err("diag: No support for SMUX device %d\n", pdev->id);
		return -EINVAL;
	}

	diag_smux[pdev->id].enabled = 1;
	return smux_open(pdev->id);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static int diagfwd_smux_remove(struct platform_device *pdev)
{
<<<<<<< HEAD
	driver->lcid = LCID_INVALID;
	driver->smux_connected = 0;
	driver->diag_smux_enabled = 0;
	driver->in_busy_smux = 1;
	kfree(driver->buf_in_smux);
	driver->buf_in_smux = NULL;
	return 0;
}

struct platform_driver msm_diagfwd_smux_driver = {
=======
	if (!pdev)
		return -EINVAL;

	pr_debug("diag: SMUX probe called, pdev->id: %d\n", pdev->id);
	if (pdev->id < 0 || pdev->id >= NUM_SMUX_DEV) {
		pr_err("diag: No support for SMUX device %d\n", pdev->id);
		return -EINVAL;
	}
	if (!diag_smux[pdev->id].enabled) {
		pr_err("diag: SMUX channel %d is not enabled\n",
		       diag_smux[pdev->id].id);
		return -ENODEV;
	}
	return smux_close(pdev->id);
}

static struct platform_driver msm_diagfwd_smux_driver = {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	.probe = diagfwd_smux_probe,
	.remove = diagfwd_smux_remove,
	.driver = {
		   .name = "SMUX_DIAG",
		   .owner = THIS_MODULE,
		   .pm   = &diagfwd_smux_dev_pm_ops,
		   },
};
<<<<<<< HEAD
=======

static struct diag_remote_dev_ops diag_smux_fwd_ops = {
	.open = smux_open,
	.close = smux_close,
	.queue_read = smux_queue_read,
	.write = smux_write,
	.fwd_complete = smux_fwd_complete,
};

int diag_smux_init()
{
	int i;
	int err = 0;
	struct diag_smux_info *ch = NULL;
	char wq_name[DIAG_SMUX_NAME_SZ + 11];

	for (i = 0; i < NUM_SMUX_DEV; i++) {
		ch = &diag_smux[i];
		strlcpy(wq_name, "DIAG_SMUX_", 11);
		strlcat(wq_name, ch->name, sizeof(ch->name));
		ch->smux_wq = create_singlethread_workqueue(wq_name);
		if (!ch->smux_wq) {
			err = -ENOMEM;
			goto fail;
		}
		err = diagfwd_bridge_register(ch->dev_id, ch->id,
					      &diag_smux_fwd_ops);
		if (err) {
			pr_err("diag: Unable to register SMUX ch %d with bridge\n",
			       ch->id);
			goto fail;
		}
	}

	err = platform_driver_register(&msm_diagfwd_smux_driver);
	if (err) {
		pr_err("diag: Unable to register SMUX device, err: %d\n", err);
		goto fail;
	}

	return 0;
fail:
	diag_smux_exit();
	return err;
}

void diag_smux_exit()
{
	int i;
	struct diag_smux_info *ch = NULL;
	for (i = 0; i < NUM_SMUX_DEV; i++) {
		ch = &diag_smux[i];
		kfree(ch->read_buf);
		ch->read_buf = NULL;
		ch->enabled = 0;
		ch->opened = 0;
		ch->read_len = 0;
	}
	platform_driver_unregister(&msm_diagfwd_smux_driver);
}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
