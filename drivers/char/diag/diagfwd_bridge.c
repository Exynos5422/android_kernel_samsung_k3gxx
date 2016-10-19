<<<<<<< HEAD
/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
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

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/diagchar.h>
#include <linux/kmemleak.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/ratelimit.h>
#include <linux/platform_device.h>
#include <linux/smux.h>
<<<<<<< HEAD
#ifdef CONFIG_DIAG_OVER_USB
#include <mach/usbdiag.h>
#endif
#include "diagchar.h"
#include "diagmem.h"
#include "diagfwd_cntl.h"
#include "diagfwd_smux.h"
#include "diagfwd_hsic.h"
#include "diag_masks.h"
#include "diagfwd_bridge.h"

struct diag_bridge_dev *diag_bridge;

/* diagfwd_connect_bridge is called when the USB mdm channel is connected */
int diagfwd_connect_bridge(int process_cable)
{
	uint8_t i;

	pr_debug("diag: in %s\n", __func__);

	for (i = 0; i < MAX_BRIDGES; i++)
		if (diag_bridge[i].enabled)
			connect_bridge(process_cable, i);
	return 0;
}

void connect_bridge(int process_cable, uint8_t index)
{
	int err;

	mutex_lock(&diag_bridge[index].bridge_mutex);
	/* If the usb cable is being connected */
	if (process_cable) {
		err = usb_diag_alloc_req(diag_bridge[index].ch, N_MDM_WRITE,
			       N_MDM_READ);
		if (err)
			pr_err("diag: unable to alloc USB req for ch %d err:%d\n",
							 index, err);

		diag_bridge[index].usb_connected = 1;
	}

	if (index == SMUX) {
		if (driver->diag_smux_enabled) {
			driver->in_busy_smux = 0;
			diagfwd_connect_smux();
		}
	} else {
		if (index >= MAX_HSIC_CH) {
			pr_err("diag: Invalid hsic channel index %d in %s\n",
							index, __func__);
			mutex_unlock(&diag_bridge[index].bridge_mutex);
			return;
		}
		if (diag_hsic[index].hsic_device_enabled &&
			(driver->logging_mode != MEMORY_DEVICE_MODE ||
			diag_hsic[index].hsic_data_requested)) {
			diag_hsic[index].in_busy_hsic_read_on_device = 0;
			diag_hsic[index].in_busy_hsic_write = 0;
			/* If the HSIC (diag_bridge) platform
			 * device is not open */
			if (!diag_hsic[index].hsic_device_opened) {
				hsic_diag_bridge_ops[index].ctxt =
							(void *)(int)(index);
				err = diag_bridge_open(index,
						 &hsic_diag_bridge_ops[index]);
				if (err) {
					pr_err("diag: HSIC channel open error: %d\n",
						   err);
				} else {
					pr_debug("diag: opened HSIC channel\n");
					diag_hsic[index].hsic_device_opened =
									1;
				}
			} else {
				pr_debug("diag: HSIC channel already open\n");
			}
			/*
			 * Turn on communication over usb mdm and HSIC,
			 * if the HSIC device driver is enabled
			 * and opened
			 */
			if (diag_hsic[index].hsic_device_opened) {
				diag_hsic[index].hsic_ch = 1;
				/* Poll USB mdm channel to check for data */
				if (driver->logging_mode == USB_MODE)
					queue_work(diag_bridge[index].wq,
						   &diag_bridge[index].
							diag_read_work);
				/* Poll HSIC channel to check for data */
				queue_work(diag_bridge[index].wq,
					   &diag_hsic[index].
					   diag_read_hsic_work);
			}
		}
	}
	mutex_unlock(&diag_bridge[index].bridge_mutex);
}

/*
 * diagfwd_disconnect_bridge is called when the USB mdm channel
 * is disconnected. So disconnect should happen for all bridges
 */
int diagfwd_disconnect_bridge(int process_cable)
{
	int i;
	pr_debug("diag: In %s, process_cable: %d\n", __func__, process_cable);

	for (i = 0; i < MAX_BRIDGES; i++) {
		if (diag_bridge[i].enabled) {
			mutex_lock(&diag_bridge[i].bridge_mutex);
			/* If the usb cable is being disconnected */
			if (process_cable) {
				diag_bridge[i].usb_connected = 0;
				usb_diag_free_req(diag_bridge[i].ch);
			}

			if (i == SMUX) {
				if (driver->diag_smux_enabled &&
					driver->logging_mode == USB_MODE) {
					driver->in_busy_smux = 1;
					driver->lcid = LCID_INVALID;
					driver->smux_connected = 0;
					/*
					 * Turn off communication over usb
					 * and smux
					 */
					msm_smux_close(LCID_VALID);
				}
			}  else {
				if (diag_hsic[i].hsic_device_enabled &&
				     (driver->logging_mode != MEMORY_DEVICE_MODE
				     || !diag_hsic[i].hsic_data_requested)) {
					diag_hsic[i].
						in_busy_hsic_read_on_device = 1;
					diag_hsic[i].in_busy_hsic_write = 1;
					/* Turn off communication over usb
					 * and HSIC */
					diag_hsic_close(i);
				}
			}
			mutex_unlock(&diag_bridge[i].bridge_mutex);
		}
	}
	return 0;
}

/* Called after the asychronous usb_diag_read() on mdm channel is complete */
int diagfwd_read_complete_bridge(struct diag_request *diag_read_ptr)
{
	 int index = (int)(diag_read_ptr->context);

	/* The read of the usb on the mdm (not HSIC/SMUX) has completed */
	diag_bridge[index].read_len = diag_read_ptr->actual;

	if (index == SMUX) {
		if (driver->diag_smux_enabled) {
			diagfwd_read_complete_smux();
			return 0;
		} else {
			pr_warning("diag: incorrect callback for smux\n");
		}
	}

	/* If SMUX not enabled, check for HSIC */
	diag_hsic[index].in_busy_hsic_read_on_device = 0;
	if (!diag_hsic[index].hsic_ch) {
		pr_err("DIAG in %s: hsic_ch == 0, ch %d\n", __func__, index);
		return 0;
	}

	/*
	 * The read of the usb driver on the mdm channel has completed.
	 * If there is no write on the HSIC in progress, check if the
	 * read has data to pass on to the HSIC. If so, pass the usb
	 * mdm data on to the HSIC.
	 */
	if (!diag_hsic[index].in_busy_hsic_write &&
		diag_bridge[index].usb_buf_out &&
		(diag_bridge[index].read_len > 0)) {

		/*
		 * Initiate the HSIC write. The HSIC write is
		 * asynchronous. When complete the write
		 * complete callback function will be called
		 */
		int err;
		diag_hsic[index].in_busy_hsic_write = 1;
		err = diag_bridge_write(index, diag_bridge[index].usb_buf_out,
					diag_bridge[index].read_len);
		if (err) {
			pr_err_ratelimited("diag: mdm data on HSIC write err: %d\n",
					err);
			/*
			 * If the error is recoverable, then clear
			 * the write flag, so we will resubmit a
			 * write on the next frame.  Otherwise, don't
			 * resubmit a write on the next frame.
			 */
			if ((-ENODEV) != err)
				diag_hsic[index].in_busy_hsic_write = 0;
		}
	}

	/*
	 * If there is no write of the usb mdm data on the
	 * HSIC channel
	 */
	if (!diag_hsic[index].in_busy_hsic_write)
		queue_work(diag_bridge[index].wq,
			 &diag_bridge[index].diag_read_work);

	return 0;
}

static void diagfwd_bridge_notifier(void *priv, unsigned event,
					struct diag_request *d_req)
{
	int index;

	switch (event) {
	case USB_DIAG_CONNECT:
		queue_work(driver->diag_wq,
			 &driver->diag_connect_work);
		break;
	case USB_DIAG_DISCONNECT:
		queue_work(driver->diag_wq,
			 &driver->diag_disconnect_work);
		break;
	case USB_DIAG_READ_DONE:
		index = (int)(d_req->context);
		queue_work(diag_bridge[index].wq,
		&diag_bridge[index].usb_read_complete_work);
		break;
	case USB_DIAG_WRITE_DONE:
		index = (int)(d_req->context);
		if (index == SMUX && driver->diag_smux_enabled)
			diagfwd_write_complete_smux();
		else if (diag_hsic[index].hsic_device_enabled)
			diagfwd_write_complete_hsic(d_req, index);
		break;
	default:
		pr_err("diag: in %s: Unknown event from USB diag:%u\n",
			__func__, event);
		break;
	}
}

void diagfwd_bridge_init(int index)
{
	int ret;
	unsigned char name[20];

	if (index == HSIC) {
		strlcpy(name, "hsic", sizeof(name));
	} else if (index == HSIC_2) {
		strlcpy(name, "hsic_2", sizeof(name));
	} else if (index == SMUX) {
		strlcpy(name, "smux", sizeof(name));
	} else {
		pr_err("diag: incorrect bridge init, instance: %d\n", index);
		return;
	}

	strlcpy(diag_bridge[index].name, name,
				sizeof(diag_bridge[index].name));
	strlcat(name, "_diag_wq", sizeof(diag_bridge[index].name));
	diag_bridge[index].id = index;
	diag_bridge[index].wq = create_singlethread_workqueue(name);
	diag_bridge[index].read_len = 0;
	diag_bridge[index].write_len = 0;
	if (diag_bridge[index].usb_buf_out == NULL)
		diag_bridge[index].usb_buf_out =
				 kzalloc(USB_MAX_OUT_BUF, GFP_KERNEL);
	if (diag_bridge[index].usb_buf_out == NULL)
		goto err;
	if (diag_bridge[index].usb_read_ptr == NULL)
		diag_bridge[index].usb_read_ptr =
			 kzalloc(sizeof(struct diag_request), GFP_KERNEL);
	if (diag_bridge[index].usb_read_ptr == NULL)
		goto err;
	if (diag_bridge[index].usb_read_ptr->context == NULL)
		diag_bridge[index].usb_read_ptr->context =
					 kzalloc(sizeof(int), GFP_KERNEL);
	if (diag_bridge[index].usb_read_ptr->context == NULL)
		goto err;
	mutex_init(&diag_bridge[index].bridge_mutex);

	if (index == HSIC || index == HSIC_2) {
		INIT_WORK(&(diag_bridge[index].usb_read_complete_work),
				 diag_usb_read_complete_hsic_fn);
#ifdef CONFIG_DIAG_OVER_USB
		INIT_WORK(&(diag_bridge[index].diag_read_work),
		      diag_read_usb_hsic_work_fn);
		if (index == HSIC)
			diag_bridge[index].ch = usb_diag_open(DIAG_MDM,
				 (void *)index, diagfwd_bridge_notifier);
		else if (index == HSIC_2)
			diag_bridge[index].ch = usb_diag_open(DIAG_MDM2,
				 (void *)index, diagfwd_bridge_notifier);
		if (IS_ERR(diag_bridge[index].ch)) {
			pr_err("diag: Unable to open USB MDM ch = %d\n", index);
			goto err;
		} else
			diag_bridge[index].enabled = 1;
#endif
	} else if (index == SMUX) {
		INIT_WORK(&(diag_bridge[index].usb_read_complete_work),
					 diag_usb_read_complete_smux_fn);
#ifdef CONFIG_DIAG_OVER_USB
		INIT_WORK(&(diag_bridge[index].diag_read_work),
					 diag_read_usb_smux_work_fn);
		diag_bridge[index].ch = usb_diag_open(DIAG_QSC, (void *)index,
					     diagfwd_bridge_notifier);
		if (IS_ERR(diag_bridge[index].ch)) {
			pr_err("diag: Unable to open USB diag QSC channel\n");
			goto err;
		} else
			diag_bridge[index].enabled = 1;
#endif
		ret = platform_driver_register(&msm_diagfwd_smux_driver);
		if (ret)
			pr_err("diag: could not register SMUX device, ret: %d\n",
									 ret);
	}
	 return;
err:
	pr_err("diag: Could not initialize for bridge forwarding\n");
	kfree(diag_bridge[index].usb_buf_out);
	kfree(diag_hsic[index].hsic_buf_tbl);
	kfree(driver->write_ptr_mdm);
	kfree(diag_bridge[index].usb_read_ptr);
	if (diag_bridge[index].wq)
		destroy_workqueue(diag_bridge[index].wq);
	return;
}

void diagfwd_bridge_exit(void)
{
	int i;
	pr_debug("diag: in %s\n", __func__);

	for (i = 0; i < MAX_HSIC_CH; i++) {
		if (diag_hsic[i].hsic_device_enabled) {
			diag_hsic_close(i);
			diag_hsic[i].hsic_device_enabled = 0;
			diag_bridge[i].enabled = 0;
		}
		diag_hsic[i].hsic_inited = 0;
		kfree(diag_hsic[i].hsic_buf_tbl);
	}
	diagmem_exit(driver, POOL_TYPE_ALL);
	if (driver->diag_smux_enabled) {
		driver->lcid = LCID_INVALID;
		kfree(driver->buf_in_smux);
		driver->diag_smux_enabled = 0;
		diag_bridge[SMUX].enabled = 0;
	}
	platform_driver_unregister(&msm_hsic_ch_driver);
	platform_driver_unregister(&msm_diagfwd_smux_driver);
	/* destroy USB MDM specific variables */
	for (i = 0; i < MAX_BRIDGES; i++) {
		if (diag_bridge[i].enabled) {
#ifdef CONFIG_DIAG_OVER_USB
			if (diag_bridge[i].usb_connected)
				usb_diag_free_req(diag_bridge[i].ch);
			usb_diag_close(diag_bridge[i].ch);
#endif
			kfree(diag_bridge[i].usb_buf_out);
			kfree(diag_bridge[i].usb_read_ptr);
			destroy_workqueue(diag_bridge[i].wq);
			diag_bridge[i].enabled = 0;
		}
	}
	kfree(driver->write_ptr_mdm);
}
=======
#include "diag_mux.h"
#include "diagfwd_bridge.h"
#include "diagfwd_hsic.h"
#include "diagfwd_smux.h"
#include "diagfwd_mhi.h"
#include "diag_dci.h"

#ifdef CONFIG_MSM_MHI
#define diag_mdm_init		diag_mhi_init
#else
#define diag_mdm_init		diag_hsic_init
#endif

#define BRIDGE_TO_MUX(x)	(x + DIAG_MUX_BRIDGE_BASE)

struct diagfwd_bridge_info bridge_info[NUM_REMOTE_DEV] = {
	{
		.id = DIAGFWD_MDM,
		.type = DIAG_DATA_TYPE,
		.name = "MDM",
		.inited = 0,
		.ctxt = 0,
		.dev_ops = NULL,
		.dci_read_ptr = NULL,
		.dci_read_buf = NULL,
		.dci_read_len = 0,
		.dci_wq = NULL,
	},
	{
		.id = DIAGFWD_SMUX,
		.type = DIAG_DATA_TYPE,
		.name = "SMUX",
		.inited = 0,
		.ctxt = 0,
		.dci_read_ptr = NULL,
		.dev_ops = NULL,
		.dci_read_buf = NULL,
		.dci_read_len = 0,
		.dci_wq = NULL,
	},
	{
		.id = DIAGFWD_MDM_DCI,
		.type = DIAG_DCI_TYPE,
		.name = "MDM_DCI",
		.inited = 0,
		.ctxt = 0,
		.dci_read_ptr = NULL,
		.dev_ops = NULL,
		.dci_read_buf = NULL,
		.dci_read_len = 0,
		.dci_wq = NULL,
	},
};

static int diagfwd_bridge_mux_connect(int id, int mode)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	bridge_info[id].dev_ops->open(bridge_info[id].ctxt);
	return 0;
}

static int diagfwd_bridge_mux_disconnect(int id, int mode)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	bridge_info[id].dev_ops->close(bridge_info[id].ctxt);
	return 0;
}

static int diagfwd_bridge_mux_read_done(unsigned char *buf, int len, int id)
{
	return diagfwd_bridge_write(id, buf, len);
}

static int diagfwd_bridge_mux_write_done(unsigned char *buf, int len,
					 int buf_ctx, int id)
{
	struct diagfwd_bridge_info *ch = NULL;

	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	ch = &bridge_info[id];
	ch->dev_ops->fwd_complete(ch->ctxt, buf, len, 0);
	return 0;
}

static struct diag_mux_ops diagfwd_bridge_mux_ops = {
	.open = diagfwd_bridge_mux_connect,
	.close = diagfwd_bridge_mux_disconnect,
	.read_done = diagfwd_bridge_mux_read_done,
	.write_done = diagfwd_bridge_mux_write_done
};

static void bridge_dci_read_work_fn(struct work_struct *work)
{
	struct diagfwd_bridge_info *ch = container_of(work,
					struct diagfwd_bridge_info,
					dci_read_work);
	if (!ch)
		return;
	diag_process_remote_dci_read_data(ch->id, ch->dci_read_buf,
					  ch->dci_read_len);
	ch->dev_ops->fwd_complete(ch->ctxt, ch->dci_read_ptr,
				  ch->dci_read_len, 0);
}

int diagfwd_bridge_register(int id, int ctxt, struct diag_remote_dev_ops *ops)
{
	int err = 0;
	struct diagfwd_bridge_info *ch = NULL;
	char wq_name[DIAG_BRIDGE_NAME_SZ + 10];

	if (!ops) {
		pr_err("diag: Invalid pointers ops: %p ctxt: %d\n", ops, ctxt);
		return -EINVAL;
	}

	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;

	ch = &bridge_info[id];
	ch->ctxt = ctxt;
	ch->dev_ops = ops;
	switch (ch->type) {
	case DIAG_DATA_TYPE:
		err = diag_mux_register(BRIDGE_TO_MUX(id), id,
					&diagfwd_bridge_mux_ops);
		if (err)
			return err;
		break;
	case DIAG_DCI_TYPE:
		ch->dci_read_buf = kzalloc(DIAG_MDM_BUF_SIZE, GFP_KERNEL);
		if (!ch->dci_read_buf)
			return -ENOMEM;
		ch->dci_read_len = 0;
		strlcpy(wq_name, "diag_dci_", 10);
		strlcat(wq_name, ch->name, sizeof(ch->name));
		INIT_WORK(&(ch->dci_read_work), bridge_dci_read_work_fn);
		ch->dci_wq = create_singlethread_workqueue(wq_name);
		if (!ch->dci_wq) {
			kfree(ch->dci_read_buf);
			return -ENOMEM;
		}
		break;
	default:
		pr_err("diag: Invalid channel type %d in %s\n", ch->type,
		       __func__);
		return -EINVAL;
	}
	return 0;
}

int diag_remote_dev_open(int id)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	bridge_info[id].inited = 1;
	if (bridge_info[id].type == DIAG_DATA_TYPE)
		return diag_mux_queue_read(BRIDGE_TO_MUX(id));
	else if (bridge_info[id].type == DIAG_DCI_TYPE)
		return diag_dci_send_handshake_pkt(bridge_info[id].id);

	return 0;
}

void diag_remote_dev_close(int id)
{
	return;
}

int diag_remote_dev_read_done(int id, unsigned char *buf, int len)
{
	int err = 0;
	struct diagfwd_bridge_info *ch = NULL;

	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	ch = &bridge_info[id];
	if (ch->type == DIAG_DATA_TYPE) {
		err = diag_mux_write(BRIDGE_TO_MUX(id), buf, len, id);
		ch->dev_ops->queue_read(ch->ctxt);
		return err;
	}
	/*
	 * For DCI channels copy to the internal buffer. Don't queue any
	 * further reads. A read should be queued once we are done processing
	 * the current packet
	 */
	if (len <= 0 || len > DIAG_MDM_BUF_SIZE) {
		pr_err_ratelimited("diag: Invalid len %d in %s, ch: %s\n",
				   len, __func__, ch->name);
		return -EINVAL;
	}
	ch->dci_read_ptr = buf;
	memcpy(ch->dci_read_buf, buf, len);
	ch->dci_read_len = len;
	queue_work(ch->dci_wq, &ch->dci_read_work);
	return 0;
}

int diag_remote_dev_write_done(int id, unsigned char *buf, int len, int ctxt)
{
	int err = 0;
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;

	if (bridge_info[id].type == DIAG_DATA_TYPE) {
		if (buf == driver->cb_buf)
			driver->cb_buf_len = 0;
		if (buf == driver->user_space_data_buf)
			driver->user_space_data_busy = 0;
		err = diag_mux_queue_read(BRIDGE_TO_MUX(id));
	} else {
		err = diag_dci_write_done_bridge(id, buf, len);
	}
	return err;
}

int diagfwd_bridge_init()
{
	int err = 0;

	err = diag_mdm_init();
	if (err)
		goto fail;
	err = diag_smux_init();
	if (err)
		goto fail;
	return 0;

fail:
	pr_err("diag: Unable to initialze diagfwd bridge, err: %d\n", err);
	return err;
}

void diagfwd_bridge_exit()
{
	diag_hsic_exit();
	diag_smux_exit();
}

int diagfwd_bridge_close(int id)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	return bridge_info[id].dev_ops->close(bridge_info[id].ctxt);
}

int diagfwd_bridge_write(int id, unsigned char *buf, int len)
{
	if (id < 0 || id >= NUM_REMOTE_DEV)
		return -EINVAL;
	return bridge_info[id].dev_ops->write(bridge_info[id].ctxt,
					      buf, len, 0);
}

uint16_t diag_get_remote_device_mask()
{
	int i;
	uint16_t remote_dev = 0;

	for (i = 0; i < NUM_REMOTE_DEV; i++) {
		if (bridge_info[i].inited)
			remote_dev |= 1 << i;

	}

	return remote_dev;
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
