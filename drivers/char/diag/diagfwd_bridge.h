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

#ifndef DIAGFWD_BRIDGE_H
#define DIAGFWD_BRIDGE_H

<<<<<<< HEAD
#include "diagfwd.h"

#define MAX_BRIDGES	5
#define HSIC	0
#define HSIC_2	1
#define SMUX	4

int diagfwd_connect_bridge(int);
void connect_bridge(int, uint8_t);
int diagfwd_disconnect_bridge(int);
void diagfwd_bridge_init(int index);
void diagfwd_bridge_exit(void);
int diagfwd_read_complete_bridge(struct diag_request *diag_read_ptr);

/* Diag-Bridge structure, n bridges can be used at same time
 * for instance SMUX, HSIC working at same time
 */
struct diag_bridge_dev {
	int id;
	char name[20];
	int enabled;
	struct mutex bridge_mutex;
	int usb_connected;
	int read_len;
	int write_len;
	unsigned char *usb_buf_out;
	struct usb_diag_ch *ch;
	struct workqueue_struct *wq;
	struct work_struct diag_read_work;
	struct diag_request *usb_read_ptr;
	struct work_struct usb_read_complete_work;
};

=======
/*
 * Add Data channels at the top half and the DCI channels at the
 * bottom half of this list.
 */
#define DIAGFWD_MDM		0
#define DIAGFWD_SMUX		1
#define NUM_REMOTE_DATA_DEV	2
#define DIAGFWD_MDM_DCI		NUM_REMOTE_DATA_DEV
#define NUM_REMOTE_DCI_DEV	(DIAGFWD_MDM_DCI - NUM_REMOTE_DATA_DEV + 1)
#define NUM_REMOTE_DEV		(NUM_REMOTE_DATA_DEV + NUM_REMOTE_DCI_DEV)

#define DIAG_BRIDGE_NAME_SZ	24
#define DIAG_BRIDGE_GET_NAME(x)	(bridge_info[x].name)

struct diag_remote_dev_ops {
	int (*open)(int id);
	int (*close)(int id);
	int (*queue_read)(int id);
	int (*write)(int id, unsigned char *buf, int len, int ctxt);
	int (*fwd_complete)(int id, unsigned char *buf, int len, int ctxt);
};

struct diagfwd_bridge_info {
	int id;
	int type;
	int inited;
	int ctxt;
	char name[DIAG_BRIDGE_NAME_SZ];
	struct diag_remote_dev_ops *dev_ops;
	/* DCI related variables. These would be NULL for data channels */
	void *dci_read_ptr;
	unsigned char *dci_read_buf;
	int dci_read_len;
	struct workqueue_struct *dci_wq;
	struct work_struct dci_read_work;
};

extern struct diagfwd_bridge_info bridge_info[NUM_REMOTE_DEV];
int diagfwd_bridge_init(void);
void diagfwd_bridge_exit(void);
int diagfwd_bridge_close(int id);
int diagfwd_bridge_write(int id, unsigned char *buf, int len);
uint16_t diag_get_remote_device_mask(void);

/* The following functions must be called by Diag remote devices only. */
int diagfwd_bridge_register(int id, int ctxt, struct diag_remote_dev_ops *ops);
int diag_remote_dev_open(int id);
void diag_remote_dev_close(int id);
int diag_remote_dev_read_done(int id, unsigned char *buf, int len);
int diag_remote_dev_write_done(int id, unsigned char *buf, int len, int ctxt);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#endif
