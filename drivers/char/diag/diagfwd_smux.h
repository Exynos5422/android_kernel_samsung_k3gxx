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

#ifndef DIAGFWD_SMUX_H
#define DIAGFWD_SMUX_H

#include <linux/smux.h>
<<<<<<< HEAD
#define LCID_VALID	SMUX_USB_DIAG_0
#define LCID_INVALID	0

int diagfwd_read_complete_smux(void);
int diagfwd_write_complete_smux(void);
int diagfwd_connect_smux(void);
void diag_usb_read_complete_smux_fn(struct work_struct *w);
void diag_read_usb_smux_work_fn(struct work_struct *work);
extern struct platform_driver msm_diagfwd_smux_driver;
=======

#define SMUX_1			0
#define NUM_SMUX_DEV		1

#define DIAG_SMUX_NAME_SZ	24

struct diag_smux_info {
	int id;
	int lcid;
	int dev_id;
	char name[DIAG_SMUX_NAME_SZ];
	unsigned char *read_buf;
	int read_len;
	int in_busy;
	int enabled;
	int inited;
	int opened;
	struct work_struct read_work;
	struct workqueue_struct *smux_wq;
};

extern struct diag_smux_info diag_smux[NUM_SMUX_DEV];

int diag_smux_init(void);
void diag_smux_exit(void);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#endif
