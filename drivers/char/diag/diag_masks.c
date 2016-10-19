<<<<<<< HEAD
/* Copyright (c) 2008-2013, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2008-2014, The Linux Foundation. All rights reserved.
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
#include <linux/workqueue.h>
<<<<<<< HEAD
=======
#include <linux/uaccess.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include "diagchar.h"
#include "diagfwd_cntl.h"
#include "diag_masks.h"

<<<<<<< HEAD
int diag_event_num_bytes;

#define DIAG_CTRL_MASK_INVALID		0
#define DIAG_CTRL_MASK_ALL_DISABLED	1
#define DIAG_CTRL_MASK_ALL_ENABLED	2
#define DIAG_CTRL_MASK_VALID		3

#define ALL_EQUIP_ID		100
#define ALL_SSID		-1

#define FEATURE_MASK_LEN_BYTES		2

struct mask_info {
	int equip_id;
	int num_items;
	int index;
};

#define CREATE_MSG_MASK_TBL_ROW(XX)					\
do {									\
	*(int *)(msg_mask_tbl_ptr) = MSG_SSID_ ## XX;			\
	msg_mask_tbl_ptr += 4;						\
	*(int *)(msg_mask_tbl_ptr) = MSG_SSID_ ## XX ## _LAST;		\
	msg_mask_tbl_ptr += 4;						\
	/* mimic the last entry as actual_last while creation */	\
	*(int *)(msg_mask_tbl_ptr) = MSG_SSID_ ## XX ## _LAST;		\
	msg_mask_tbl_ptr += 4;						\
	/* increment by MAX_SSID_PER_RANGE cells */			\
	msg_mask_tbl_ptr += MAX_SSID_PER_RANGE * sizeof(int);		\
} while (0)

static void diag_print_mask_table(void)
{
/* Enable this to print mask table when updated */
#ifdef MASK_DEBUG
	int first, last, actual_last;
	uint8_t *ptr = driver->msg_masks;
	int i = 0;
	pr_info("diag: F3 message mask table\n");
	while (*(uint32_t *)(ptr + 4)) {
		first = *(uint32_t *)ptr;
		ptr += 4;
		last = *(uint32_t *)ptr;
		ptr += 4;
		actual_last = *(uint32_t *)ptr;
		ptr += 4;
		pr_info("diag: SSID %d, %d - %d\n", first, last, actual_last);
		for (i = 0 ; i <= actual_last - first ; i++)
			pr_info("diag: MASK:%x\n", *((uint32_t *)ptr + i));
		ptr += MAX_SSID_PER_RANGE*4;
	}
#endif
}

void diag_create_msg_mask_table(void)
{
	uint8_t *msg_mask_tbl_ptr = driver->msg_masks;

	CREATE_MSG_MASK_TBL_ROW(0);
	CREATE_MSG_MASK_TBL_ROW(1);
	CREATE_MSG_MASK_TBL_ROW(2);
	CREATE_MSG_MASK_TBL_ROW(3);
	CREATE_MSG_MASK_TBL_ROW(4);
	CREATE_MSG_MASK_TBL_ROW(5);
	CREATE_MSG_MASK_TBL_ROW(6);
	CREATE_MSG_MASK_TBL_ROW(7);
	CREATE_MSG_MASK_TBL_ROW(8);
	CREATE_MSG_MASK_TBL_ROW(9);
	CREATE_MSG_MASK_TBL_ROW(10);
	CREATE_MSG_MASK_TBL_ROW(11);
	CREATE_MSG_MASK_TBL_ROW(12);
	CREATE_MSG_MASK_TBL_ROW(13);
	CREATE_MSG_MASK_TBL_ROW(14);
	CREATE_MSG_MASK_TBL_ROW(15);
	CREATE_MSG_MASK_TBL_ROW(16);
	CREATE_MSG_MASK_TBL_ROW(17);
	CREATE_MSG_MASK_TBL_ROW(18);
	CREATE_MSG_MASK_TBL_ROW(19);
	CREATE_MSG_MASK_TBL_ROW(20);
	CREATE_MSG_MASK_TBL_ROW(21);
	CREATE_MSG_MASK_TBL_ROW(22);
	CREATE_MSG_MASK_TBL_ROW(23);
}

static void diag_set_msg_mask(int rt_mask)
{
	int first_ssid, last_ssid, i;
	uint8_t *parse_ptr, *ptr = driver->msg_masks;

	mutex_lock(&driver->diagchar_mutex);
	driver->msg_status = rt_mask ? DIAG_CTRL_MASK_ALL_ENABLED :
						DIAG_CTRL_MASK_ALL_DISABLED;
	while (*(uint32_t *)(ptr + 4)) {
		first_ssid = *(uint32_t *)ptr;
		ptr += 8; /* increment by 8 to skip 'last' */
		last_ssid = *(uint32_t *)ptr;
		ptr += 4;
		parse_ptr = ptr;
		pr_debug("diag: updating range %d %d\n", first_ssid, last_ssid);
		for (i = 0; i < last_ssid - first_ssid + 1; i++) {
			*(int *)parse_ptr = rt_mask;
			parse_ptr += 4;
		}
		ptr += MAX_SSID_PER_RANGE * 4;
	}
	mutex_unlock(&driver->diagchar_mutex);
}

static void diag_update_msg_mask(int start, int end , uint8_t *buf)
{
	int found = 0, first, last, actual_last;
	uint8_t *actual_last_ptr;
	uint8_t *ptr = driver->msg_masks;
	uint8_t *ptr_buffer_start = &(*(driver->msg_masks));
	uint8_t *ptr_buffer_end = &(*(driver->msg_masks)) + MSG_MASK_SIZE;
	uint32_t copy_len = (end - start + 1) * sizeof(int);

	mutex_lock(&driver->diagchar_mutex);
	/* First SSID can be zero : So check that last is non-zero */
	while (*(uint32_t *)(ptr + 4)) {
		first = *(uint32_t *)ptr;
		ptr += 4;
		last = *(uint32_t *)ptr;
		ptr += 4;
		actual_last = *(uint32_t *)ptr;
		actual_last_ptr = ptr;
		ptr += 4;
		if (start >= first && start <= actual_last) {
			ptr += (start - first)*4;
			if (end > actual_last) {
				pr_info("diag: ssid range mismatch\n");
				actual_last = end;
				*(uint32_t *)(actual_last_ptr) = end;
			}
			if (actual_last-first >= MAX_SSID_PER_RANGE) {
				pr_err("diag: In %s, truncating ssid range, %d-%d to max allowed: %d",
						__func__, first, actual_last,
						MAX_SSID_PER_RANGE);
				copy_len = MAX_SSID_PER_RANGE;
				actual_last = first + MAX_SSID_PER_RANGE;
				*(uint32_t *)actual_last_ptr = actual_last;
			}
			if (CHK_OVERFLOW(ptr_buffer_start, ptr, ptr_buffer_end,
								copy_len)) {
				pr_debug("diag: update ssid start %d, end %d\n",
								 start, end);
				memcpy(ptr, buf, copy_len);
			} else
				pr_alert("diag: Not enough space MSG_MASK\n");
			found = 1;
			break;
		} else {
			ptr += MAX_SSID_PER_RANGE*4;
		}
	}
	/* Entry was not found - add new table */
	if (!found) {
		if (CHK_OVERFLOW(ptr_buffer_start, ptr, ptr_buffer_end,
				  8 + ((end - start) + 1)*4)) {
			memcpy(ptr, &(start) , 4);
			ptr += 4;
			memcpy(ptr, &(end), 4);
			ptr += 4;
			memcpy(ptr, &(end), 4); /* create actual_last entry */
			ptr += 4;
			pr_debug("diag: adding NEW ssid start %d, end %d\n",
								 start, end);
			memcpy(ptr, buf , ((end - start) + 1)*4);
		} else
			pr_alert("diag: Not enough buffer space for MSG_MASK\n");
	}
	driver->msg_status = DIAG_CTRL_MASK_VALID;
	mutex_unlock(&driver->diagchar_mutex);
	diag_print_mask_table();
}

void diag_toggle_event_mask(int toggle)
{
	uint8_t *ptr = driver->event_masks;

	mutex_lock(&driver->diagchar_mutex);
	if (toggle) {
		driver->event_status = DIAG_CTRL_MASK_ALL_ENABLED;
		memset(ptr, 0xFF, EVENT_MASK_SIZE);
	} else {
		driver->event_status = DIAG_CTRL_MASK_ALL_DISABLED;
		memset(ptr, 0, EVENT_MASK_SIZE);
	}
	mutex_unlock(&driver->diagchar_mutex);
}


static void diag_update_event_mask(uint8_t *buf, int num_bytes)
{
	uint8_t *ptr = driver->event_masks;
	uint8_t *temp = buf + 2;

	mutex_lock(&driver->diagchar_mutex);
	if (CHK_OVERFLOW(ptr, ptr, ptr+EVENT_MASK_SIZE, num_bytes)) {
		memcpy(ptr, temp, num_bytes);
		driver->event_status = DIAG_CTRL_MASK_VALID;
	} else {
		pr_err("diag: In %s, not enough buffer space\n", __func__);
	}
	mutex_unlock(&driver->diagchar_mutex);
}

static void diag_disable_log_mask(void)
{
	int i = 0;
	struct mask_info *parse_ptr = (struct mask_info *)(driver->log_masks);

	pr_debug("diag: disable log masks\n");
	mutex_lock(&driver->diagchar_mutex);
	for (i = 0; i < MAX_EQUIP_ID; i++) {
		pr_debug("diag: equip id %d\n", parse_ptr->equip_id);
		if (!(parse_ptr->equip_id)) /* Reached a null entry */
			break;
		memset(driver->log_masks + parse_ptr->index, 0,
			    (parse_ptr->num_items + 7)/8);
		parse_ptr++;
	}
	driver->log_status = DIAG_CTRL_MASK_ALL_DISABLED;
	mutex_unlock(&driver->diagchar_mutex);
}

int chk_equip_id_and_mask(int equip_id, uint8_t *buf)
{
	int i = 0, flag = 0, num_items, offset;
	unsigned char *ptr_data;
	struct mask_info *ptr = (struct mask_info *)(driver->log_masks);

	pr_debug("diag: received equip id = %d\n", equip_id);
	/* Check if this is valid equipment ID */
	for (i = 0; i < MAX_EQUIP_ID; i++) {
		if ((ptr->equip_id == equip_id) && (ptr->index != 0)) {
			offset = ptr->index;
			num_items = ptr->num_items;
			flag = 1;
			break;
		}
		ptr++;
	}
	if (!flag)
		return -EPERM;
	ptr_data = driver->log_masks + offset;
	memcpy(buf, ptr_data, (num_items+7)/8);
	return 0;
}

static void diag_update_log_mask(int equip_id, uint8_t *buf, int num_items)
{
	uint8_t *temp = buf;
	int i = 0;
	unsigned char *ptr_data;
	int offset = (sizeof(struct mask_info))*MAX_EQUIP_ID;
	struct mask_info *ptr = (struct mask_info *)(driver->log_masks);

	pr_debug("diag: received equip id = %d\n", equip_id);
	mutex_lock(&driver->diagchar_mutex);
	/* Check if we already know index of this equipment ID */
	for (i = 0; i < MAX_EQUIP_ID; i++) {
		if ((ptr->equip_id == equip_id) && (ptr->index != 0)) {
			offset = ptr->index;
			break;
		}
		if ((ptr->equip_id == 0) && (ptr->index == 0)) {
			/* Reached a null entry */
			ptr->equip_id = equip_id;
			ptr->num_items = num_items;
			ptr->index = driver->log_masks_length;
			offset = driver->log_masks_length;
			driver->log_masks_length += ((num_items+7)/8);
			break;
		}
		ptr++;
	}
	ptr_data = driver->log_masks + offset;
	if (CHK_OVERFLOW(driver->log_masks, ptr_data, driver->log_masks
					 + LOG_MASK_SIZE, (num_items+7)/8)) {
		memcpy(ptr_data, temp, (num_items+7)/8);
		driver->log_status = DIAG_CTRL_MASK_VALID;
	} else {
		pr_err("diag: Not enough buffer space for LOG_MASK\n");
		driver->log_status = DIAG_CTRL_MASK_INVALID;
	}
	mutex_unlock(&driver->diagchar_mutex);
}

void diag_mask_update_fn(struct work_struct *work)
{
	struct diag_smd_info *smd_info = container_of(work,
						struct diag_smd_info,
						diag_notify_update_smd_work);
	if (!smd_info) {
		pr_err("diag: In %s, smd info is null, cannot update masks for the peripheral\n",
			__func__);
		return;
	}

	diag_send_feature_mask_update(smd_info);
	diag_send_msg_mask_update(smd_info->ch, ALL_SSID, ALL_SSID,
						smd_info->peripheral);
	diag_send_log_mask_update(smd_info->ch, ALL_EQUIP_ID);
	diag_send_event_mask_update(smd_info->ch, diag_event_num_bytes);

	if (smd_info->notify_context == SMD_EVENT_OPEN)
		diag_send_diag_mode_update_by_smd(smd_info,
						driver->real_time_mode);

	smd_info->notify_context = 0;
}

void diag_send_log_mask_update(smd_channel_t *ch, int equip_id)
{
	void *buf = driver->buf_log_mask_update;
	int header_size = sizeof(struct diag_ctrl_log_mask);
	struct mask_info *ptr = (struct mask_info *)driver->log_masks;
	int i, size, wr_size = -ENOMEM, retry_count = 0;

	mutex_lock(&driver->diag_cntl_mutex);
	for (i = 0; i < MAX_EQUIP_ID; i++) {
		size = (ptr->num_items+7)/8;
		/* reached null entry */
		if ((ptr->equip_id == 0) && (ptr->index == 0))
			break;
		driver->log_mask->cmd_type = DIAG_CTRL_MSG_LOG_MASK;
		driver->log_mask->num_items = ptr->num_items;
		driver->log_mask->data_len  = 11 + size;
		driver->log_mask->stream_id = 1; /* 2, if dual stream */
		driver->log_mask->equip_id = ptr->equip_id;
		driver->log_mask->status = driver->log_status;
		switch (driver->log_status) {
		case DIAG_CTRL_MASK_ALL_DISABLED:
			driver->log_mask->log_mask_size = 0;
			break;
		case DIAG_CTRL_MASK_ALL_ENABLED:
			driver->log_mask->log_mask_size = 0;
			break;
		case DIAG_CTRL_MASK_VALID:
			driver->log_mask->log_mask_size = size;
			break;
		default:
			/* Log status is not set or the buffer is corrupted */
			pr_err("diag: In %s, invalid status %d", __func__,
							driver->log_status);
			driver->log_mask->status = DIAG_CTRL_MASK_INVALID;
		}

		if (driver->log_mask->status == DIAG_CTRL_MASK_INVALID) {
			mutex_unlock(&driver->diag_cntl_mutex);
			return;
		}
		/* send only desired update, NOT ALL */
		if (equip_id == ALL_EQUIP_ID || equip_id ==
					 driver->log_mask->equip_id) {
			memcpy(buf, driver->log_mask, header_size);
			if (driver->log_status == DIAG_CTRL_MASK_VALID)
				memcpy(buf + header_size,
				       driver->log_masks+ptr->index, size);
			if (ch) {
				while (retry_count < 3) {
					wr_size = smd_write(ch, buf,
							 header_size + size);
					if (wr_size == -ENOMEM) {
						retry_count++;
						usleep_range(10000, 10100);
					} else
						break;
				}
				if (wr_size != header_size + size)
					pr_err("diag: log mask update failed %d, tried %d",
						wr_size, header_size + size);
				else
					pr_debug("diag: updated log equip ID %d,len %d\n",
					driver->log_mask->equip_id,
					driver->log_mask->log_mask_size);
			} else
				pr_err("diag: ch not valid for log update\n");
		}
		ptr++;
	}
	mutex_unlock(&driver->diag_cntl_mutex);
}

void diag_send_event_mask_update(smd_channel_t *ch, int num_bytes)
{
	void *buf = driver->buf_event_mask_update;
	int header_size = sizeof(struct diag_ctrl_event_mask);
	int wr_size = -ENOMEM, retry_count = 0;

	mutex_lock(&driver->diag_cntl_mutex);
	if (num_bytes == 0) {
		pr_debug("diag: event mask not set yet, so no update\n");
		mutex_unlock(&driver->diag_cntl_mutex);
		return;
	}
	/* send event mask update */
	driver->event_mask->cmd_type = DIAG_CTRL_MSG_EVENT_MASK;
	driver->event_mask->data_len = 7 + num_bytes;
	driver->event_mask->stream_id = 1; /* 2, if dual stream */
	driver->event_mask->status = driver->event_status;

	switch (driver->event_status) {
	case DIAG_CTRL_MASK_ALL_DISABLED:
		driver->event_mask->event_config = 0;
		driver->event_mask->event_mask_size = 0;
		break;
	case DIAG_CTRL_MASK_ALL_ENABLED:
		driver->event_mask->event_config = 1;
		driver->event_mask->event_mask_size = 0;
		break;
	case DIAG_CTRL_MASK_VALID:
		driver->event_mask->event_config = 1;
		driver->event_mask->event_mask_size = num_bytes;
		memcpy(buf + header_size, driver->event_masks, num_bytes);
		break;
	default:
		/* Event status is not set yet or the buffer is corrupted */
		pr_err("diag: In %s, invalid status %d", __func__,
							driver->event_status);
		driver->event_mask->status = DIAG_CTRL_MASK_INVALID;
	}

	if (driver->event_mask->status == DIAG_CTRL_MASK_INVALID) {
		mutex_unlock(&driver->diag_cntl_mutex);
		return;
	}
	memcpy(buf, driver->event_mask, header_size);
	if (ch) {
		while (retry_count < 3) {
			wr_size = smd_write(ch, buf, header_size + num_bytes);
			if (wr_size == -ENOMEM) {
				retry_count++;
				usleep_range(10000, 10100);
			} else
				break;
		}
		if (wr_size != header_size + num_bytes)
			pr_err("diag: error writing event mask %d, tried %d\n",
					 wr_size, header_size + num_bytes);
	} else
		pr_err("diag: ch not valid for event update\n");
	mutex_unlock(&driver->diag_cntl_mutex);
}

void diag_send_msg_mask_update(smd_channel_t *ch, int updated_ssid_first,
						int updated_ssid_last, int proc)
{
	void *buf = driver->buf_msg_mask_update;
	int first, last, actual_last, size = -ENOMEM, retry_count = 0;
	int header_size = sizeof(struct diag_ctrl_msg_mask);
	uint8_t *ptr = driver->msg_masks;

	mutex_lock(&driver->diag_cntl_mutex);
	while (*(uint32_t *)(ptr + 4)) {
		first = *(uint32_t *)ptr;
		ptr += 4;
		last = *(uint32_t *)ptr;
		ptr += 4;
		actual_last = *(uint32_t *)ptr;
		ptr += 4;
		if (!((updated_ssid_first >= first && updated_ssid_last <=
			 actual_last) || (updated_ssid_first == ALL_SSID))) {
			ptr += MAX_SSID_PER_RANGE*4;
			continue;
		}
		/* send f3 mask update */
		driver->msg_mask->cmd_type = DIAG_CTRL_MSG_F3_MASK;
		driver->msg_mask->status = driver->msg_status;
		switch (driver->msg_status) {
		case DIAG_CTRL_MASK_ALL_DISABLED:
			driver->msg_mask->msg_mask_size = 0;
			break;
		case DIAG_CTRL_MASK_ALL_ENABLED:
			driver->msg_mask->msg_mask_size = 1;
			memcpy(buf+header_size, ptr,
				 4 * (driver->msg_mask->msg_mask_size));
			break;
		case DIAG_CTRL_MASK_VALID:
			driver->msg_mask->msg_mask_size = actual_last -
								first + 1;
			/* Limit the msg_mask_size to MAX_SSID_PER_RANGE */
			if (driver->msg_mask->msg_mask_size >
							MAX_SSID_PER_RANGE) {
				pr_err("diag: in %s, Invalid msg mask size %d, max: %d",
					__func__,
				       driver->msg_mask->msg_mask_size,
				       MAX_SSID_PER_RANGE);
				driver->msg_mask->msg_mask_size =
							MAX_SSID_PER_RANGE;
			}
			memcpy(buf+header_size, ptr,
				 4 * (driver->msg_mask->msg_mask_size));
			break;
		default:
			/* Msg status is not set or the buffer is corrupted */
			pr_err("diag: In %s, invalid status %d", __func__,
							driver->msg_status);
			driver->msg_mask->status = DIAG_CTRL_MASK_INVALID;
		}

		if (driver->msg_mask->status == DIAG_CTRL_MASK_INVALID) {
			mutex_unlock(&driver->diag_cntl_mutex);
			return;
		}
		driver->msg_mask->data_len = 11 +
					4 * (driver->msg_mask->msg_mask_size);
		driver->msg_mask->stream_id = 1; /* 2, if dual stream */
		driver->msg_mask->msg_mode = 0; /* Legcay mode */
		driver->msg_mask->ssid_first = first;
		driver->msg_mask->ssid_last = actual_last;
		memcpy(buf, driver->msg_mask, header_size);
		if (ch) {
			while (retry_count < 3) {
				size = smd_write(ch, buf, header_size +
				 4*(driver->msg_mask->msg_mask_size));
				if (size == -ENOMEM) {
					retry_count++;
					usleep_range(10000, 10100);
				} else
					break;
			}
			if (size != header_size +
				 4*(driver->msg_mask->msg_mask_size))
				pr_err("diag: proc %d, msg mask update fail %d, tried %d\n",
					proc, size, (header_size +
				4*(driver->msg_mask->msg_mask_size)));
			else
				pr_debug("diag: sending mask update for ssid first %d, last %d on PROC %d\n",
					first, actual_last, proc);
		} else
			pr_err("diag: proc %d, ch invalid msg mask update\n",
								proc);
		ptr += MAX_SSID_PER_RANGE*4;
	}
	mutex_unlock(&driver->diag_cntl_mutex);
}

void diag_send_feature_mask_update(struct diag_smd_info *smd_info)
{
	void *buf = driver->buf_feature_mask_update;
	int header_size = sizeof(struct diag_ctrl_feature_mask);
	int wr_size = -ENOMEM, retry_count = 0;
	uint8_t feature_bytes[FEATURE_MASK_LEN_BYTES] = {0, 0};
	int total_len = 0;

	if (!smd_info) {
		pr_err("diag: In %s, null smd info pointer\n",
			__func__);
		return;
	}

	if (!smd_info->ch) {
		pr_err("diag: In %s, smd channel not open for peripheral: %d, type: %d\n",
				__func__, smd_info->peripheral, smd_info->type);
		return;
	}

	mutex_lock(&driver->diag_cntl_mutex);
	/* send feature mask update */
	driver->feature_mask->ctrl_pkt_id = DIAG_CTRL_MSG_FEATURE;
	driver->feature_mask->ctrl_pkt_data_len = 4 + FEATURE_MASK_LEN_BYTES;
	driver->feature_mask->feature_mask_len = FEATURE_MASK_LEN_BYTES;
	memcpy(buf, driver->feature_mask, header_size);
	feature_bytes[0] |= F_DIAG_INT_FEATURE_MASK;
	feature_bytes[0] |= F_DIAG_LOG_ON_DEMAND_RSP_ON_MASTER;
	feature_bytes[0] |= driver->supports_separate_cmdrsp ?
				F_DIAG_REQ_RSP_CHANNEL : 0;
	feature_bytes[0] |= driver->supports_apps_hdlc_encoding ?
				F_DIAG_HDLC_ENCODE_IN_APPS_MASK : 0;
	feature_bytes[1] |= F_DIAG_OVER_STM;
	memcpy(buf+header_size, &feature_bytes, FEATURE_MASK_LEN_BYTES);
	total_len = header_size + FEATURE_MASK_LEN_BYTES;

	while (retry_count < 3) {
		wr_size = smd_write(smd_info->ch, buf, total_len);
		if (wr_size == -ENOMEM) {
			retry_count++;
			/*
			 * The smd channel is full. Delay while
			 * smd processes existing data and smd
			 * has memory become available. The delay
			 * of 10000 was determined empirically as
			 * best value to use.
			 */
			usleep_range(10000, 10100);
		} else
			break;
	}
	if (wr_size != total_len)
		pr_err("diag: In %s, peripheral %d fail feature update, size: %d, tried: %d",
			__func__, smd_info->peripheral, wr_size, total_len);

	mutex_unlock(&driver->diag_cntl_mutex);
}

int diag_process_apps_masks(unsigned char *buf, int len)
{
	int packet_type = 1;
	int i;
	int ssid_first, ssid_last, ssid_range;
	int rt_mask, rt_first_ssid, rt_last_ssid, rt_mask_size;
	uint8_t *rt_mask_ptr;
	int equip_id, num_items;
#if defined(CONFIG_DIAG_OVER_USB)
	int payload_length;
#endif

	/* Set log masks */
	if (*buf == 0x73 && *(int *)(buf+4) == 3) {
		buf += 8;
		/* Read Equip ID and pass as first param below*/
		diag_update_log_mask(*(int *)buf, buf+8, *(int *)(buf+4));
		diag_update_userspace_clients(LOG_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x73;
			*(int *)(driver->apps_rsp_buf + 4) = 0x3; /* op. ID */
			*(int *)(driver->apps_rsp_buf + 8) = 0x0; /* success */
			payload_length = 8 + ((*(int *)(buf + 4)) + 7)/8;
			if (payload_length > APPS_BUF_SIZE - 12) {
				pr_err("diag: log masks: buffer overflow\n");
				return -EIO;
			}
			for (i = 0; i < payload_length; i++)
				*(int *)(driver->apps_rsp_buf+12+i) = *(buf+i);

			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_log_mask_update(
						driver->smd_cntl[i].ch,
						*(int *)buf);
			}
			encode_rsp_and_send(12 + payload_length - 1);
			return 0;
		}
#endif
	} /* Get log masks */
	else if (*buf == 0x73 && *(int *)(buf+4) == 4) {
#if defined(CONFIG_DIAG_OVER_USB)
		if (!(driver->smd_data[MODEM_DATA].ch) &&
						chk_apps_only()) {
			equip_id = *(int *)(buf + 8);
			num_items = *(int *)(buf + 12);
			driver->apps_rsp_buf[0] = 0x73;
			driver->apps_rsp_buf[1] = 0x0;
			driver->apps_rsp_buf[2] = 0x0;
			driver->apps_rsp_buf[3] = 0x0;
			*(int *)(driver->apps_rsp_buf + 4) = 0x4;
			if (!chk_equip_id_and_mask(equip_id,
				driver->apps_rsp_buf+20))
				*(int *)(driver->apps_rsp_buf + 8) = 0x0;
			else
				*(int *)(driver->apps_rsp_buf + 8) = 0x1;
			*(int *)(driver->apps_rsp_buf + 12) = equip_id;
			*(int *)(driver->apps_rsp_buf + 16) = num_items;
			encode_rsp_and_send(20+(num_items+7)/8-1);
			return 0;
		}
#endif
	} /* Disable log masks */
	else if (*buf == 0x73 && *(int *)(buf+4) == 0) {
		/* Disable mask for each log code */
		diag_disable_log_mask();
		diag_update_userspace_clients(LOG_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x73;
			driver->apps_rsp_buf[1] = 0x0;
			driver->apps_rsp_buf[2] = 0x0;
			driver->apps_rsp_buf[3] = 0x0;
			*(int *)(driver->apps_rsp_buf + 4) = 0x0;
			*(int *)(driver->apps_rsp_buf + 8) = 0x0; /* status */
			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_log_mask_update(
						driver->smd_cntl[i].ch,
						ALL_EQUIP_ID);

			}
			encode_rsp_and_send(11);
			return 0;
		}
#endif
	} /* Get runtime message mask  */
	else if ((*buf == 0x7d) && (*(buf+1) == 0x3)) {
		ssid_first = *(uint16_t *)(buf + 2);
		ssid_last = *(uint16_t *)(buf + 4);
#if defined(CONFIG_DIAG_OVER_USB)
		if (!(driver->smd_data[MODEM_DATA].ch) &&
						chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x7d;
			driver->apps_rsp_buf[1] = 0x3;
			*(uint16_t *)(driver->apps_rsp_buf+2) = ssid_first;
			*(uint16_t *)(driver->apps_rsp_buf+4) = ssid_last;
			driver->apps_rsp_buf[6] = 0x1; /* Success Status */
			driver->apps_rsp_buf[7] = 0x0;
			rt_mask_ptr = driver->msg_masks;
			while (*(uint32_t *)(rt_mask_ptr + 4)) {
				rt_first_ssid = *(uint32_t *)rt_mask_ptr;
				rt_mask_ptr += 8; /* +8 to skip 'last' */
				rt_last_ssid = *(uint32_t *)rt_mask_ptr;
				rt_mask_ptr += 4;
				if (ssid_first == rt_first_ssid && ssid_last ==
					rt_last_ssid) {
					rt_mask_size = 4 * (rt_last_ssid -
						rt_first_ssid + 1);
					if (rt_mask_size > APPS_BUF_SIZE - 8) {
						pr_err("diag: rt masks: buffer overflow\n");
						return -EIO;
					}
					memcpy(driver->apps_rsp_buf+8,
						rt_mask_ptr, rt_mask_size);
					encode_rsp_and_send(8+rt_mask_size-1);
					return 0;
				}
				rt_mask_ptr += MAX_SSID_PER_RANGE*4;
			}
		}
#endif
	} /* Set runtime message mask  */
	else if ((*buf == 0x7d) && (*(buf+1) == 0x4)) {
		ssid_first = *(uint16_t *)(buf + 2);
		ssid_last = *(uint16_t *)(buf + 4);
		if (ssid_last < ssid_first) {
			pr_err("diag: Invalid msg mask ssid values, first: %d, last: %d\n",
				ssid_first, ssid_last);
			return -EIO;
		}
		ssid_range = 4 * (ssid_last - ssid_first + 1);
		if (ssid_range > APPS_BUF_SIZE - 8) {
			pr_err("diag: Not enough space for message mask, ssid_range: %d\n",
				ssid_range);
			return -EIO;
		}
		pr_debug("diag: received mask update for ssid_first = %d, ssid_last = %d",
			ssid_first, ssid_last);
		diag_update_msg_mask(ssid_first, ssid_last , buf + 8);
		diag_update_userspace_clients(MSG_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			for (i = 0; i < 8 + ssid_range; i++)
				*(driver->apps_rsp_buf + i) = *(buf+i);
			*(driver->apps_rsp_buf + 6) = 0x1;
			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_msg_mask_update(
						driver->smd_cntl[i].ch,
						ssid_first, ssid_last,
						driver->smd_cntl[i].peripheral);

			}
			encode_rsp_and_send(8 + ssid_range - 1);
			return 0;
		}
#endif
	} /* Set ALL runtime message mask  */
	else if ((*buf == 0x7d) && (*(buf+1) == 0x5)) {
		rt_mask = *(int *)(buf + 4);
		diag_set_msg_mask(rt_mask);
		diag_update_userspace_clients(MSG_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x7d; /* cmd_code */
			driver->apps_rsp_buf[1] = 0x5; /* set subcommand */
			driver->apps_rsp_buf[2] = 1; /* success */
			driver->apps_rsp_buf[3] = 0; /* rsvd */
			*(int *)(driver->apps_rsp_buf + 4) = rt_mask;
			/* send msg mask update to peripheral */
			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_msg_mask_update(
						driver->smd_cntl[i].ch,
						ALL_SSID, ALL_SSID,
						driver->smd_cntl[i].peripheral);

			}
			encode_rsp_and_send(7);
			return 0;
		}
#endif
	} else if (*buf == 0x82) {	/* event mask change */
		buf += 4;
		diag_event_num_bytes = (*(uint16_t *)buf)/8+1;
		diag_update_event_mask(buf, diag_event_num_bytes);
		diag_update_userspace_clients(EVENT_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x82;
			driver->apps_rsp_buf[1] = 0x0;
			*(uint16_t *)(driver->apps_rsp_buf + 2) = 0x0;
			*(uint16_t *)(driver->apps_rsp_buf + 4) =
				EVENT_LAST_ID + 1;
			memcpy(driver->apps_rsp_buf+6, driver->event_masks,
				EVENT_LAST_ID/8+1);
			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_event_mask_update(
						driver->smd_cntl[i].ch,
						diag_event_num_bytes);
			}
			encode_rsp_and_send(6 + EVENT_LAST_ID/8);
			return 0;
		}
#endif
	} else if (*buf == 0x60) {
		diag_toggle_event_mask(*(buf+1));
		diag_update_userspace_clients(EVENT_MASKS_TYPE);
#if defined(CONFIG_DIAG_OVER_USB)
		if (chk_apps_only()) {
			driver->apps_rsp_buf[0] = 0x60;
			driver->apps_rsp_buf[1] = 0x0;
			driver->apps_rsp_buf[2] = 0x0;
			for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
				if (driver->smd_cntl[i].ch)
					diag_send_event_mask_update(
						driver->smd_cntl[i].ch,
						diag_event_num_bytes);
			}
			encode_rsp_and_send(2);
			return 0;
		}
#endif
	} else if (*buf == 0x78) {
		if (!(driver->smd_cntl[MODEM_DATA].ch) ||
					(driver->log_on_demand_support)) {
			driver->apps_rsp_buf[0] = 0x78;
			/* Copy log code received */
			*(uint16_t *)(driver->apps_rsp_buf + 1) =
							*(uint16_t *)(buf + 1);
			driver->apps_rsp_buf[3] = 0x1;/* Unknown */
			encode_rsp_and_send(3);
		}
	}

	return  packet_type;
}

void diag_masks_init(void)
{
	driver->event_status = DIAG_CTRL_MASK_INVALID;
	driver->msg_status = DIAG_CTRL_MASK_INVALID;
	driver->log_status = DIAG_CTRL_MASK_INVALID;

	if (driver->event_mask == NULL) {
		driver->event_mask = kzalloc(sizeof(
			struct diag_ctrl_event_mask), GFP_KERNEL);
		if (driver->event_mask == NULL)
			goto err;
		kmemleak_not_leak(driver->event_mask);
	}
	if (driver->msg_mask == NULL) {
		driver->msg_mask = kzalloc(sizeof(
			struct diag_ctrl_msg_mask), GFP_KERNEL);
		if (driver->msg_mask == NULL)
			goto err;
		kmemleak_not_leak(driver->msg_mask);
	}
	if (driver->log_mask == NULL) {
		driver->log_mask = kzalloc(sizeof(
			struct diag_ctrl_log_mask), GFP_KERNEL);
		if (driver->log_mask == NULL)
			goto err;
		kmemleak_not_leak(driver->log_mask);
	}

	if (driver->buf_msg_mask_update == NULL) {
		driver->buf_msg_mask_update = kzalloc(APPS_BUF_SIZE,
								 GFP_KERNEL);
		if (driver->buf_msg_mask_update == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_msg_mask_update);
	}
	if (driver->buf_log_mask_update == NULL) {
		driver->buf_log_mask_update = kzalloc(APPS_BUF_SIZE,
								 GFP_KERNEL);
		if (driver->buf_log_mask_update == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_log_mask_update);
	}
	if (driver->buf_event_mask_update == NULL) {
		driver->buf_event_mask_update = kzalloc(APPS_BUF_SIZE,
								 GFP_KERNEL);
		if (driver->buf_event_mask_update == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_event_mask_update);
	}
	if (driver->msg_masks == NULL) {
		driver->msg_masks = kzalloc(MSG_MASK_SIZE, GFP_KERNEL);
		if (driver->msg_masks == NULL)
			goto err;
		kmemleak_not_leak(driver->msg_masks);
	}
	if (driver->buf_feature_mask_update == NULL) {
		driver->buf_feature_mask_update = kzalloc(sizeof(
					struct diag_ctrl_feature_mask) +
					FEATURE_MASK_LEN_BYTES, GFP_KERNEL);
		if (driver->buf_feature_mask_update == NULL)
			goto err;
		kmemleak_not_leak(driver->buf_feature_mask_update);
	}
	if (driver->feature_mask == NULL) {
		driver->feature_mask = kzalloc(sizeof(
			struct diag_ctrl_feature_mask), GFP_KERNEL);
		if (driver->feature_mask == NULL)
			goto err;
		kmemleak_not_leak(driver->feature_mask);
	}
	diag_create_msg_mask_table();
	diag_event_num_bytes = 0;
	if (driver->log_masks == NULL) {
		driver->log_masks = kzalloc(LOG_MASK_SIZE, GFP_KERNEL);
		if (driver->log_masks == NULL)
			goto err;
		kmemleak_not_leak(driver->log_masks);
	}
	driver->log_masks_length = (sizeof(struct mask_info))*MAX_EQUIP_ID;
	if (driver->event_masks == NULL) {
		driver->event_masks = kzalloc(EVENT_MASK_SIZE, GFP_KERNEL);
		if (driver->event_masks == NULL)
			goto err;
		kmemleak_not_leak(driver->event_masks);
	}
	return;
err:
	pr_err("diag: Could not initialize diag mask buffers");
	kfree(driver->event_mask);
	kfree(driver->log_mask);
	kfree(driver->msg_mask);
	kfree(driver->msg_masks);
	kfree(driver->log_masks);
	kfree(driver->event_masks);
	kfree(driver->feature_mask);
	kfree(driver->buf_feature_mask_update);
=======
#define ALL_EQUIP_ID		100
#define ALL_SSID		-1

#define DIAG_SET_FEATURE_MASK(x) (feature_bytes[(x)/8] |= (1 << (x & 0x7)))

struct diag_mask_info msg_mask;
struct diag_mask_info msg_bt_mask;
struct diag_mask_info log_mask;
struct diag_mask_info event_mask;

static const struct diag_ssid_range_t msg_mask_tbl[] = {
	{ .ssid_first = MSG_SSID_0, .ssid_last = MSG_SSID_0_LAST },
	{ .ssid_first = MSG_SSID_1, .ssid_last = MSG_SSID_1_LAST },
	{ .ssid_first = MSG_SSID_2, .ssid_last = MSG_SSID_2_LAST },
	{ .ssid_first = MSG_SSID_3, .ssid_last = MSG_SSID_3_LAST },
	{ .ssid_first = MSG_SSID_4, .ssid_last = MSG_SSID_4_LAST },
	{ .ssid_first = MSG_SSID_5, .ssid_last = MSG_SSID_5_LAST },
	{ .ssid_first = MSG_SSID_6, .ssid_last = MSG_SSID_6_LAST },
	{ .ssid_first = MSG_SSID_7, .ssid_last = MSG_SSID_7_LAST },
	{ .ssid_first = MSG_SSID_8, .ssid_last = MSG_SSID_8_LAST },
	{ .ssid_first = MSG_SSID_9, .ssid_last = MSG_SSID_9_LAST },
	{ .ssid_first = MSG_SSID_10, .ssid_last = MSG_SSID_10_LAST },
	{ .ssid_first = MSG_SSID_11, .ssid_last = MSG_SSID_11_LAST },
	{ .ssid_first = MSG_SSID_12, .ssid_last = MSG_SSID_12_LAST },
	{ .ssid_first = MSG_SSID_13, .ssid_last = MSG_SSID_13_LAST },
	{ .ssid_first = MSG_SSID_14, .ssid_last = MSG_SSID_14_LAST },
	{ .ssid_first = MSG_SSID_15, .ssid_last = MSG_SSID_15_LAST },
	{ .ssid_first = MSG_SSID_16, .ssid_last = MSG_SSID_16_LAST },
	{ .ssid_first = MSG_SSID_17, .ssid_last = MSG_SSID_17_LAST },
	{ .ssid_first = MSG_SSID_18, .ssid_last = MSG_SSID_18_LAST },
	{ .ssid_first = MSG_SSID_19, .ssid_last = MSG_SSID_19_LAST },
	{ .ssid_first = MSG_SSID_20, .ssid_last = MSG_SSID_20_LAST },
	{ .ssid_first = MSG_SSID_21, .ssid_last = MSG_SSID_21_LAST },
	{ .ssid_first = MSG_SSID_22, .ssid_last = MSG_SSID_22_LAST },
	{ .ssid_first = MSG_SSID_23, .ssid_last = MSG_SSID_23_LAST },
	{ .ssid_first = MSG_SSID_24, .ssid_last = MSG_SSID_24_LAST }
};

static int diag_apps_responds(void)
{
	/*
	 * Apps processor should respond to mask commands only if the
	 * Modem channel is up, the feature mask is received from Modem
	 * and if Modem supports Mask Centralization.
	 */
	if (chk_apps_only()) {
		if (driver->smd_data[MODEM_DATA].ch &&
			driver->rcvd_feature_mask[MODEM_DATA]) {
			if (driver->mask_centralization[MODEM_DATA])
				return 1;
			return 0;
		}
		return 1;
	}
	return 0;
}

static void diag_send_log_mask_update(struct diag_smd_info *smd_info,
				      int equip_id)
{
	int i;
	int err = 0;
	int send_once = 0;
	int header_len = sizeof(struct diag_ctrl_log_mask);
	uint8_t *buf = log_mask.update_buf;
	uint8_t *temp = NULL;
	uint32_t mask_size = 0;
	struct diag_ctrl_log_mask ctrl_pkt;
	struct diag_log_mask_t *mask = (struct diag_log_mask_t *)log_mask.ptr;

	if (!smd_info)
		return;

	if (!smd_info->ch) {
		pr_debug("diag: In %s, SMD channel is closed, peripheral: %d\n",
			 __func__, smd_info->peripheral);
		return;
	}

	switch (log_mask.status) {
	case DIAG_CTRL_MASK_ALL_DISABLED:
		ctrl_pkt.equip_id = 0;
		ctrl_pkt.num_items = 0;
		ctrl_pkt.log_mask_size = 0;
		send_once = 1;
		break;
	case DIAG_CTRL_MASK_ALL_ENABLED:
		ctrl_pkt.equip_id = 0;
		ctrl_pkt.num_items = 0;
		ctrl_pkt.log_mask_size = 0;
		send_once = 1;
		break;
	case DIAG_CTRL_MASK_VALID:
		send_once = 0;
		break;
	default:
		pr_debug("diag: In %s, invalid log_mask status\n", __func__);
		return;
	}

	mutex_lock(&log_mask.lock);
	for (i = 0; i < MAX_EQUIP_ID; i++, mask++) {
		if (equip_id != i && equip_id != ALL_EQUIP_ID)
			continue;

		ctrl_pkt.cmd_type = DIAG_CTRL_MSG_LOG_MASK;
		ctrl_pkt.stream_id = 1;
		ctrl_pkt.status = log_mask.status;
		if (log_mask.status == DIAG_CTRL_MASK_VALID) {
			mask_size = LOG_ITEMS_TO_SIZE(mask->num_items);
			ctrl_pkt.equip_id = i;
			ctrl_pkt.num_items = mask->num_items;
			ctrl_pkt.log_mask_size = mask_size;
		}
		ctrl_pkt.data_len = LOG_MASK_CTRL_HEADER_LEN + mask_size;

		if (header_len + mask_size > log_mask.update_buf_len) {
			temp = krealloc(buf, header_len + mask_size,
					GFP_KERNEL);
			if (!temp) {
				pr_err("diag: Unable to realloc log update buffer, new size: %d, equip_id: %d\n",
				       header_len + mask_size, equip_id);
				break;
			}
			log_mask.update_buf = temp;
			log_mask.update_buf_len = header_len + mask_size;
		}

		memcpy(buf, &ctrl_pkt, header_len);
		if (mask_size > 0)
			memcpy(buf + header_len, mask->ptr, mask_size);

		err = diag_smd_write(smd_info, buf, header_len + mask_size);
		if (err) {
			pr_err("diag: Unable to send log masks to peripheral %d, equip_id: %d, err: %d\n",
			       smd_info->peripheral, i, err);
		}
		if (send_once || equip_id != ALL_EQUIP_ID)
			break;

	}
	mutex_unlock(&log_mask.lock);
}

static void diag_send_event_mask_update(struct diag_smd_info *smd_info)
{
	uint8_t *buf = event_mask.update_buf;
	uint8_t *temp = NULL;
	struct diag_ctrl_event_mask header;
	int num_bytes = EVENT_COUNT_TO_BYTES(driver->last_event_id);
	int write_len = 0;
	int err = 0;
	int temp_len = 0;

	if (num_bytes <= 0 || num_bytes > driver->event_mask_size) {
		pr_debug("diag: In %s, invalid event mask length %d\n",
			 __func__, num_bytes);
		return;
	}

	if (!smd_info)
		return;

	if (!smd_info->ch) {
		pr_debug("diag: In %s, SMD channel is closed, peripheral: %d\n",
			 __func__, smd_info->peripheral);
		return;
	}

	mutex_lock(&event_mask.lock);
	header.cmd_type = DIAG_CTRL_MSG_EVENT_MASK;
	header.stream_id = 1;
	header.status = event_mask.status;

	switch (event_mask.status) {
	case DIAG_CTRL_MASK_ALL_DISABLED:
		header.event_config = 0;
		header.event_mask_size = 0;
		break;
	case DIAG_CTRL_MASK_ALL_ENABLED:
		header.event_config = 1;
		header.event_mask_size = 0;
		break;
	case DIAG_CTRL_MASK_VALID:
		header.event_config = 1;
		header.event_mask_size = num_bytes;
		if (num_bytes + sizeof(header) > event_mask.update_buf_len) {
			temp_len = num_bytes + sizeof(header);
			temp = krealloc(buf, temp_len, GFP_KERNEL);
			if (!temp) {
				pr_err("diag: Unable to realloc event mask update buffer\n");
				goto err;
			} else {
				event_mask.update_buf = temp;
				event_mask.update_buf_len = temp_len;
			}
		}
		memcpy(buf + sizeof(header), event_mask.ptr, num_bytes);
		write_len += num_bytes;
		break;
	default:
		pr_debug("diag: In %s, invalid status %d\n", __func__,
			 event_mask.status);
		goto err;
	}
	header.data_len = EVENT_MASK_CTRL_HEADER_LEN + header.event_mask_size;
	memcpy(buf, &header, sizeof(header));
	write_len += sizeof(header);

	err = diag_smd_write(smd_info, buf, write_len);
	if (err) {
		pr_err("diag: Unable to send event masks to peripheral %d\n",
		       smd_info->peripheral);
	}
err:
	mutex_unlock(&event_mask.lock);
}

static void diag_send_msg_mask_update(struct diag_smd_info *smd_info,
				      int first, int last)
{
	int i;
	int err = 0;
	int header_len = sizeof(struct diag_ctrl_msg_mask);
	int temp_len = 0;
	uint8_t *buf = msg_mask.update_buf;
	uint8_t *temp = NULL;
	uint32_t mask_size = 0;
	struct diag_msg_mask_t *mask = (struct diag_msg_mask_t *)msg_mask.ptr;
	struct diag_ctrl_msg_mask header;

	if (!smd_info)
		return;

	if (!smd_info->ch) {
		pr_debug("diag: In %s, SMD channel is closed, peripheral: %d\n",
			 __func__, smd_info->peripheral);
		return;
	}

	mutex_lock(&msg_mask.lock);
	switch (msg_mask.status) {
	case DIAG_CTRL_MASK_ALL_DISABLED:
		mask_size = 0;
		break;
	case DIAG_CTRL_MASK_ALL_ENABLED:
		mask_size = 1;
		break;
	case DIAG_CTRL_MASK_VALID:
		break;
	default:
		pr_debug("diag: In %s, invalid status: %d\n", __func__,
			 msg_mask.status);
		goto err;
	}

	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		if (((first < mask->ssid_first) || (last > mask->ssid_last)) &&
							first != ALL_SSID) {
			continue;
		}

		if (msg_mask.status == DIAG_CTRL_MASK_VALID) {
			mask_size = mask->ssid_last - mask->ssid_first + 1;
			temp_len = mask_size * sizeof(uint32_t);
			if (temp_len + header_len <= msg_mask.update_buf_len)
				goto proceed;
			temp = krealloc(msg_mask.update_buf, temp_len,
					GFP_KERNEL);
			if (!temp) {
				pr_err("diag: In %s, unable to realloc msg_mask update buffer\n",
				       __func__);
				mask_size = (msg_mask.update_buf_len -
					    header_len) / sizeof(uint32_t);
			} else {
				msg_mask.update_buf = temp;
				msg_mask.update_buf_len = temp_len;
				pr_debug("diag: In %s, successfully reallocated msg_mask update buffer to len: %d\n",
					 __func__, msg_mask.update_buf_len);
			}
		}
proceed:
		header.cmd_type = DIAG_CTRL_MSG_F3_MASK;
		header.status = msg_mask.status;
		header.stream_id = 1;
		header.msg_mode = 0;
		header.ssid_first = mask->ssid_first;
		header.ssid_last = mask->ssid_last;
		header.msg_mask_size = mask_size;
		mask_size *= sizeof(uint32_t);
		header.data_len = MSG_MASK_CTRL_HEADER_LEN + mask_size;
		memcpy(buf, &header, header_len);
		if (mask_size > 0)
			memcpy(buf + header_len, mask->ptr, mask_size);

		err = diag_smd_write(smd_info, buf, header_len + mask_size);
		if (err) {
			pr_err("diag: Unable to send msg masks to peripheral %d\n",
			       smd_info->peripheral);
		}

		if (first != ALL_SSID)
			break;
	}
err:
	mutex_unlock(&msg_mask.lock);
}

static void diag_send_feature_mask_update(struct diag_smd_info *smd_info)
{
	void *buf = driver->buf_feature_mask_update;
	int header_size = sizeof(struct diag_ctrl_feature_mask);
	uint8_t feature_bytes[FEATURE_MASK_LEN] = {0, 0};
	struct diag_ctrl_feature_mask feature_mask;
	int total_len = 0;
	int err = 0;

	if (!smd_info) {
		pr_err("diag: In %s, null smd info pointer\n",
			__func__);
		return;
	}

	if (!smd_info->ch) {
		pr_err("diag: In %s, smd channel not open for peripheral: %d, type: %d\n",
				__func__, smd_info->peripheral, smd_info->type);
		return;
	}

	mutex_lock(&driver->diag_cntl_mutex);
	/* send feature mask update */
	feature_mask.ctrl_pkt_id = DIAG_CTRL_MSG_FEATURE;
	feature_mask.ctrl_pkt_data_len = sizeof(uint32_t) + FEATURE_MASK_LEN;
	feature_mask.feature_mask_len = FEATURE_MASK_LEN;
	memcpy(buf, &feature_mask, header_size);
	DIAG_SET_FEATURE_MASK(F_DIAG_FEATURE_MASK_SUPPORT);
	DIAG_SET_FEATURE_MASK(F_DIAG_LOG_ON_DEMAND_APPS);
	DIAG_SET_FEATURE_MASK(F_DIAG_STM);
	if (driver->supports_separate_cmdrsp)
		DIAG_SET_FEATURE_MASK(F_DIAG_REQ_RSP_SUPPORT);
	if (driver->supports_apps_hdlc_encoding)
		DIAG_SET_FEATURE_MASK(F_DIAG_APPS_HDLC_ENCODE);
	DIAG_SET_FEATURE_MASK(F_DIAG_MASK_CENTRALIZATION);
	memcpy(buf + header_size, &feature_bytes, FEATURE_MASK_LEN);
	total_len = header_size + FEATURE_MASK_LEN;

	err = diag_smd_write(smd_info, buf, total_len);
	if (err) {
		pr_err("diag: In %s, unable to write to smd, peripheral: %d, type: %d, len: %d, err: %d\n",
		       __func__, smd_info->peripheral, smd_info->type,
		       total_len, err);
	}
	mutex_unlock(&driver->diag_cntl_mutex);
}

static int diag_cmd_get_ssid_range(unsigned char *src_buf, int src_len,
				   unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	struct diag_msg_mask_t *mask_ptr = NULL;
	struct diag_msg_ssid_query_t rsp;
	struct diag_ssid_range_t ssid_range;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	if (!diag_apps_responds())
		return 0;

	rsp.cmd_code = DIAG_CMD_MSG_CONFIG;
	rsp.sub_cmd = DIAG_CMD_OP_GET_SSID_RANGE;
	rsp.status = MSG_STATUS_SUCCESS;
	rsp.padding = 0;
	rsp.count = driver->msg_mask_tbl_count;
	memcpy(dest_buf, &rsp, sizeof(rsp));
	write_len += sizeof(rsp);

	mask_ptr = (struct diag_msg_mask_t *)msg_mask.ptr;
	for (i = 0; i <  driver->msg_mask_tbl_count; i++, mask_ptr++) {
		if (write_len + sizeof(ssid_range) > dest_len) {
			pr_err("diag: In %s, Truncating response due to size limitations of rsp buffer\n",
			       __func__);
			break;
		}
		ssid_range.ssid_first = mask_ptr->ssid_first;
		ssid_range.ssid_last = mask_ptr->ssid_last;
		memcpy(dest_buf + write_len, &ssid_range, sizeof(ssid_range));
		write_len += sizeof(ssid_range);
	}

	return write_len;
}

static int diag_cmd_get_build_mask(unsigned char *src_buf, int src_len,
				   unsigned char *dest_buf, int dest_len)
{
	int i = 0;
	int write_len = 0;
	int num_entries = 0;
	int copy_len = 0;
	struct diag_msg_mask_t *build_mask = NULL;
	struct diag_build_mask_req_t *req = NULL;
	struct diag_msg_build_mask_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	if (!diag_apps_responds())
		return 0;

	req = (struct diag_build_mask_req_t *)src_buf;
	rsp.cmd_code = DIAG_CMD_MSG_CONFIG;
	rsp.sub_cmd = DIAG_CMD_OP_GET_BUILD_MASK;
	rsp.ssid_first = req->ssid_first;
	rsp.ssid_last = req->ssid_last;
	rsp.status = MSG_STATUS_FAIL;
	rsp.padding = 0;

	build_mask = (struct diag_msg_mask_t *)msg_bt_mask.ptr;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, build_mask++) {
		if (build_mask->ssid_first != req->ssid_first)
			continue;
		num_entries = req->ssid_last - req->ssid_first + 1;
		if (num_entries > build_mask->range) {
			pr_warn("diag: In %s, truncating ssid range for ssid_first: %d ssid_last %d\n",
				__func__, req->ssid_first, req->ssid_last);
			num_entries = build_mask->range;
			req->ssid_last = req->ssid_first + build_mask->range;
		}
		copy_len = num_entries * sizeof(uint32_t);
		if (copy_len + sizeof(rsp) > dest_len)
			copy_len = dest_len - sizeof(rsp);
		memcpy(dest_buf + sizeof(rsp), build_mask->ptr, copy_len);
		write_len += copy_len;
		rsp.ssid_last = build_mask->ssid_last;
		rsp.status = MSG_STATUS_SUCCESS;
		break;
	}
	memcpy(dest_buf, &rsp, sizeof(rsp));
	write_len += sizeof(rsp);

	return write_len;
}

static int diag_cmd_get_msg_mask(unsigned char *src_buf, int src_len,
				 unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	uint32_t mask_size = 0;
	struct diag_msg_mask_t *mask = NULL;
	struct diag_build_mask_req_t *req = NULL;
	struct diag_msg_build_mask_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	if (!diag_apps_responds())
		return 0;

	req = (struct diag_build_mask_req_t *)src_buf;
	rsp.cmd_code = DIAG_CMD_MSG_CONFIG;
	rsp.sub_cmd = DIAG_CMD_OP_GET_MSG_MASK;
	rsp.ssid_first = req->ssid_first;
	rsp.ssid_last = req->ssid_last;
	rsp.status = MSG_STATUS_FAIL;
	rsp.padding = 0;

	mask = (struct diag_msg_mask_t *)msg_mask.ptr;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		if ((req->ssid_first < mask->ssid_first) ||
		    (req->ssid_first > mask->ssid_last)) {
			continue;
		}
		mask_size = mask->range * sizeof(uint32_t);
		/* Copy msg mask only till the end of the rsp buffer */
		if (mask_size + sizeof(rsp) > dest_len)
			mask_size = dest_len - sizeof(rsp);
		memcpy(dest_buf + sizeof(rsp), mask->ptr, mask_size);
		write_len += mask_size;
		rsp.status = MSG_STATUS_SUCCESS;
		break;
	}
	memcpy(dest_buf, &rsp, sizeof(rsp));
	write_len += sizeof(rsp);

	return write_len;
}

static int diag_cmd_set_msg_mask(unsigned char *src_buf, int src_len,
				 unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	int header_len = sizeof(struct diag_msg_build_mask_t);
	int found = 0;
	uint32_t mask_size = 0;
	uint32_t offset = 0;
	struct diag_msg_mask_t *mask = NULL;
	struct diag_msg_build_mask_t *req = NULL;
	struct diag_msg_build_mask_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	req = (struct diag_msg_build_mask_t *)src_buf;

	mutex_lock(&msg_mask.lock);
	mask = (struct diag_msg_mask_t *)msg_mask.ptr;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		if ((req->ssid_first < mask->ssid_first) ||
		    (req->ssid_first > mask->ssid_last)) {
			continue;
		}
		found = 1;
		if (req->ssid_last > mask->ssid_last) {
			pr_debug("diag: Msg SSID range mismatch\n");
			mask->ssid_last = req->ssid_last;
		}
		mask_size = req->ssid_last - req->ssid_first + 1;
		if (mask_size > mask->range) {
			pr_warn("diag: In %s, truncating ssid range, %d-%d to max allowed: %d\n",
				__func__, mask->ssid_first, mask->ssid_last,
				mask->range);
			mask_size = mask->range;
			mask->ssid_last = mask->ssid_first + mask->range;
		}
		offset = req->ssid_first - mask->ssid_first;
		if (offset + mask_size > mask->range) {
			pr_err("diag: In %s, Not enough space for msg mask, mask_size: %d\n",
			       __func__, mask_size);
			break;
		}
		mask_size = mask_size * sizeof(uint32_t);
		memcpy(mask->ptr + offset, src_buf + header_len, mask_size);
		msg_mask.status = DIAG_CTRL_MASK_VALID;
		break;
	}
	mutex_unlock(&msg_mask.lock);

	diag_update_userspace_clients(MSG_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	rsp.cmd_code = DIAG_CMD_MSG_CONFIG;
	rsp.sub_cmd = DIAG_CMD_OP_SET_MSG_MASK;
	rsp.ssid_first = req->ssid_first;
	rsp.ssid_last = req->ssid_last;
	rsp.status = found;
	rsp.padding = 0;
	memcpy(dest_buf, &rsp, header_len);
	write_len += header_len;
	if (!found)
		goto end;
	if (mask_size + write_len > dest_len)
		mask_size = dest_len - write_len;
	memcpy(dest_buf + write_len, src_buf + header_len, mask_size);
	write_len += mask_size;
	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
		diag_send_msg_mask_update(&driver->smd_cntl[i],
					  req->ssid_first, req->ssid_last);
	}
end:
	return write_len;
}

static int diag_cmd_set_all_msg_mask(unsigned char *src_buf, int src_len,
				     unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	int header_len = sizeof(struct diag_msg_config_rsp_t);
	struct diag_msg_config_rsp_t rsp;
	struct diag_msg_config_rsp_t *req = NULL;
	struct diag_msg_mask_t *mask = (struct diag_msg_mask_t *)msg_mask.ptr;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	req = (struct diag_msg_config_rsp_t *)src_buf;

	mutex_lock(&msg_mask.lock);
	msg_mask.status = (req->rt_mask) ? DIAG_CTRL_MASK_ALL_ENABLED :
					   DIAG_CTRL_MASK_ALL_DISABLED;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		memset(mask->ptr, req->rt_mask,
		       mask->range * sizeof(uint32_t));
	}
	mutex_unlock(&msg_mask.lock);

	diag_update_userspace_clients(MSG_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	rsp.cmd_code = DIAG_CMD_MSG_CONFIG;
	rsp.sub_cmd = DIAG_CMD_OP_SET_ALL_MSG_MASK;
	rsp.status = MSG_STATUS_SUCCESS;
	rsp.padding = 0;
	rsp.rt_mask = req->rt_mask;
	memcpy(dest_buf, &rsp, header_len);
	write_len += header_len;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
		diag_send_msg_mask_update(&driver->smd_cntl[i], ALL_SSID,
					  ALL_SSID);
	}

	return write_len;
}

static int diag_cmd_get_event_mask(unsigned char *src_buf, int src_len,
				   unsigned char *dest_buf, int dest_len)
{
	int write_len = 0;
	uint32_t mask_size;
	struct diag_event_mask_config_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	if (!diag_apps_responds())
		return 0;

	mask_size = EVENT_COUNT_TO_BYTES(driver->last_event_id);
	if (mask_size + sizeof(rsp) > dest_len) {
		pr_err("diag: In %s, invalid mask size: %d\n", __func__,
		       mask_size);
		return -ENOMEM;
	}

	rsp.cmd_code = DIAG_CMD_GET_EVENT_MASK;
	rsp.status = EVENT_STATUS_SUCCESS;
	rsp.padding = 0;
	rsp.num_bits = driver->last_event_id + 1;
	memcpy(dest_buf, &rsp, sizeof(rsp));
	write_len += sizeof(rsp);
	memcpy(dest_buf + write_len, event_mask.ptr, mask_size);
	write_len += mask_size;

	return write_len;
}

static int diag_cmd_update_event_mask(unsigned char *src_buf, int src_len,
				      unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	int mask_len = 0;
	int header_len = sizeof(struct diag_event_mask_config_t);
	struct diag_event_mask_config_t rsp;
	struct diag_event_mask_config_t *req;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	req = (struct diag_event_mask_config_t *)src_buf;
	mask_len = EVENT_COUNT_TO_BYTES(req->num_bits);
	if (mask_len <= 0 || mask_len > event_mask.mask_len) {
		pr_err("diag: In %s, invalid event mask len: %d\n", __func__,
		       mask_len);
		return -EIO;
	}

	mutex_lock(&event_mask.lock);
	memcpy(event_mask.ptr, src_buf + header_len, mask_len);
	event_mask.status = DIAG_CTRL_MASK_VALID;
	mutex_unlock(&event_mask.lock);
	diag_update_userspace_clients(EVENT_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	rsp.cmd_code = DIAG_CMD_SET_EVENT_MASK;
	rsp.status = EVENT_STATUS_SUCCESS;
	rsp.padding = 0;
	rsp.num_bits = driver->last_event_id + 1;
	memcpy(dest_buf, &rsp, header_len);
	write_len += header_len;
	memcpy(dest_buf + write_len, event_mask.ptr, mask_len);
	write_len += mask_len;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		diag_send_event_mask_update(&driver->smd_cntl[i]);

	return write_len;
}

static int diag_cmd_toggle_events(unsigned char *src_buf, int src_len,
				  unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	uint8_t toggle = 0;
	struct diag_event_report_t header;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	toggle = *(src_buf + 1);
	mutex_lock(&event_mask.lock);
	if (toggle) {
		event_mask.status = DIAG_CTRL_MASK_ALL_ENABLED;
		memset(event_mask.ptr, 0xFF, event_mask.mask_len);
	} else {
		event_mask.status = DIAG_CTRL_MASK_ALL_DISABLED;
		memset(event_mask.ptr, 0, event_mask.mask_len);
	}
	mutex_unlock(&event_mask.lock);
	diag_update_userspace_clients(EVENT_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	header.cmd_code = DIAG_CMD_EVENT_TOGGLE;
	header.padding = 0;
	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		diag_send_event_mask_update(&driver->smd_cntl[i]);
	memcpy(dest_buf, &header, sizeof(header));
	write_len += sizeof(header);

	return write_len;
}

static int diag_cmd_get_log_mask(unsigned char *src_buf, int src_len,
				 unsigned char *dest_buf, int dest_len)
{
	int i;
	int status = LOG_STATUS_INVALID;
	int write_len = 0;
	int read_len = 0;
	int req_header_len = sizeof(struct diag_log_config_req_t);
	int rsp_header_len = sizeof(struct diag_log_config_rsp_t);
	uint32_t mask_size = 0;
	struct diag_log_mask_t *log_item = NULL;
	struct diag_log_config_req_t *req;
	struct diag_log_config_rsp_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	if (!diag_apps_responds())
		return 0;

	req = (struct diag_log_config_req_t *)src_buf;
	read_len += req_header_len;

	rsp.cmd_code = DIAG_CMD_LOG_CONFIG;
	rsp.padding[0] = 0;
	rsp.padding[1] = 0;
	rsp.padding[2] = 0;
	rsp.sub_cmd = DIAG_CMD_OP_GET_LOG_MASK;
	/*
	 * Don't copy the response header now. Copy at the end after
	 * calculating the status field value
	 */
	write_len += rsp_header_len;

	log_item = (struct diag_log_mask_t *)log_mask.ptr;
	for (i = 0; i < MAX_EQUIP_ID; i++, log_item++) {
		if (log_item->equip_id != req->equip_id)
			continue;
		mask_size = LOG_ITEMS_TO_SIZE(log_item->num_items);
		/*
		 * Make sure we have space to fill the response in the buffer.
		 * Destination buffer should atleast be able to hold equip_id
		 * (uint32_t), num_items(uint32_t), mask (mask_size) and the
		 * response header.
		 */
		if ((mask_size + (2 * sizeof(uint32_t)) + rsp_header_len) >
								dest_len) {
			pr_err("diag: In %s, invalid length: %d, max rsp_len: %d\n",
				__func__, mask_size, dest_len);
			status = LOG_STATUS_FAIL;
			break;
		}
		*(uint32_t *)(dest_buf + write_len) = log_item->equip_id;
		write_len += sizeof(uint32_t);
		*(uint32_t *)(dest_buf + write_len) = log_item->num_items;
		write_len += sizeof(uint32_t);
		if (mask_size > 0) {
			memcpy(dest_buf + write_len, log_item->ptr, mask_size);
			write_len += mask_size;
		}
		status = LOG_STATUS_SUCCESS;
		break;
	}

	rsp.status = status;
	memcpy(dest_buf, &rsp, rsp_header_len);

	return write_len;
}

static int diag_cmd_get_log_range(unsigned char *src_buf, int src_len,
				  unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	struct diag_log_config_rsp_t rsp;
	struct diag_log_mask_t *mask = (struct diag_log_mask_t *)log_mask.ptr;

	if (!diag_apps_responds())
		return 0;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	rsp.cmd_code = DIAG_CMD_LOG_CONFIG;
	rsp.padding[0] = 0;
	rsp.padding[1] = 0;
	rsp.padding[2] = 0;
	rsp.sub_cmd = DIAG_CMD_OP_GET_LOG_RANGE;
	rsp.status = LOG_STATUS_SUCCESS;
	memcpy(dest_buf, &rsp, sizeof(rsp));
	write_len += sizeof(rsp);

	for (i = 0; i < MAX_EQUIP_ID && write_len < dest_len; i++, mask++) {
		*(uint32_t *)(dest_buf + write_len) = mask->num_items;
		write_len += sizeof(uint32_t);
	}

	return write_len;
}

static int diag_cmd_set_log_mask(unsigned char *src_buf, int src_len,
				 unsigned char *dest_buf, int dest_len)
{
	int i;
	int write_len = 0;
	int status = LOG_STATUS_SUCCESS;
	int read_len = 0;
	int payload_len = 0;
	int req_header_len = sizeof(struct diag_log_config_req_t);
	int rsp_header_len = sizeof(struct diag_log_config_set_rsp_t);
	uint32_t mask_size = 0;
	struct diag_log_mask_t *mask = (struct diag_log_mask_t *)log_mask.ptr;
	struct diag_log_config_req_t *req;
	struct diag_log_config_set_rsp_t rsp;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	req = (struct diag_log_config_req_t *)src_buf;
	read_len += req_header_len;

	if (req->equip_id >= MAX_EQUIP_ID) {
		pr_err("diag: In %s, Invalid logging mask request, equip_id: %d\n",
		       __func__, req->equip_id);
		status = LOG_STATUS_INVALID;
	}
	mutex_lock(&log_mask.lock);
	for (i = 0; i < MAX_EQUIP_ID && !status; i++, mask++) {
		if (mask->equip_id != req->equip_id)
			continue;
		if (req->num_items < mask->num_items)
			mask->num_items = req->num_items;
		mask_size = LOG_ITEMS_TO_SIZE(req->num_items);
		if (mask_size > mask->range) {
			/*
			 * If the size of the log mask cannot fit into our
			 * buffer, trim till we have space left in the buffer.
			 * num_items should then reflect the items that we have
			 * in our buffer.
			 */
			mask_size = mask->range;
			mask->num_items = LOG_SIZE_TO_ITEMS(mask_size);
			req->num_items = mask->num_items;
		}
		if (mask_size > 0)
			memcpy(mask->ptr, src_buf + read_len, mask_size);
		log_mask.status = DIAG_CTRL_MASK_VALID;
		break;
	}
	mutex_unlock(&log_mask.lock);
	diag_update_userspace_clients(LOG_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	payload_len = LOG_ITEMS_TO_SIZE(req->num_items);
	if (payload_len + rsp_header_len > dest_len) {
		pr_err("diag: In %s, invalid length, payload_len: %d, header_len: %d, dest_len: %d\n",
		       __func__, payload_len, rsp_header_len , dest_len);
		status = LOG_STATUS_FAIL;
	}
	rsp.cmd_code = DIAG_CMD_LOG_CONFIG;
	rsp.padding[0] = 0;
	rsp.padding[1] = 0;
	rsp.padding[2] = 0;
	rsp.sub_cmd = DIAG_CMD_OP_SET_LOG_MASK;
	rsp.status = status;
	rsp.equip_id = req->equip_id;
	rsp.num_items = req->num_items;
	memcpy(dest_buf, &rsp, rsp_header_len);
	write_len += rsp_header_len;
	if (status != LOG_STATUS_SUCCESS)
		goto end;
	memcpy(dest_buf + write_len, src_buf + read_len, payload_len);
	write_len += payload_len;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		diag_send_log_mask_update(&driver->smd_cntl[i], req->equip_id);
end:
	return write_len;
}

static int diag_cmd_disable_log_mask(unsigned char *src_buf, int src_len,
				     unsigned char *dest_buf, int dest_len)
{
	struct diag_log_mask_t *mask = (struct diag_log_mask_t *)log_mask.ptr;
	struct diag_log_config_rsp_t header;
	int write_len = 0;
	int i;

	if (!src_buf || !dest_buf || src_len <= 0 || dest_len <= 0) {
		pr_err("diag: Invalid input in %s, src_buf: %p, src_len: %d, dest_buf: %p, dest_len: %d",
		       __func__, src_buf, src_len, dest_buf, dest_len);
		return -EINVAL;
	}

	mutex_lock(&log_mask.lock);
	for (i = 0; i < MAX_EQUIP_ID; i++, mask++)
		memset(mask->ptr, 0, mask->range);
	log_mask.status = DIAG_CTRL_MASK_ALL_DISABLED;
	mutex_unlock(&log_mask.lock);
	diag_update_userspace_clients(LOG_MASKS_TYPE);

	/*
	 * Apps processor must send the response to this command. Frame the
	 * response.
	 */
	header.cmd_code = DIAG_CMD_LOG_CONFIG;
	header.padding[0] = 0;
	header.padding[1] = 0;
	header.padding[2] = 0;
	header.sub_cmd = DIAG_CMD_OP_LOG_DISABLE;
	header.status = LOG_STATUS_SUCCESS;
	memcpy(dest_buf, &header, sizeof(struct diag_log_config_rsp_t));
	write_len += sizeof(struct diag_log_config_rsp_t);
	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		diag_send_log_mask_update(&driver->smd_cntl[i], ALL_EQUIP_ID);

	return write_len;
}

int diag_create_msg_mask_table_entry(struct diag_msg_mask_t *msg_mask,
				     struct diag_ssid_range_t *range)
{
	if (!msg_mask || !range)
		return -EIO;
	if (range->ssid_last < range->ssid_first)
		return -EINVAL;
	msg_mask->ssid_first = range->ssid_first;
	msg_mask->ssid_last = range->ssid_last;
	msg_mask->range = msg_mask->ssid_last - msg_mask->ssid_first + 1;
	if (msg_mask->range < MAX_SSID_PER_RANGE)
		msg_mask->range = MAX_SSID_PER_RANGE;
	if (msg_mask->range > 0) {
		msg_mask->ptr = kzalloc(msg_mask->range * sizeof(uint32_t),
					GFP_KERNEL);
		if (!msg_mask->ptr)
			return -ENOMEM;
		kmemleak_not_leak(msg_mask->ptr);
	}
	return 0;
}

static int diag_create_msg_mask_table(void)
{
	int i;
	int err = 0;
	struct diag_msg_mask_t *mask = (struct diag_msg_mask_t *)msg_mask.ptr;
	struct diag_ssid_range_t range;

	mutex_lock(&msg_mask.lock);
	driver->msg_mask_tbl_count = MSG_MASK_TBL_CNT;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		range.ssid_first = msg_mask_tbl[i].ssid_first;
		range.ssid_last = msg_mask_tbl[i].ssid_last;
		err = diag_create_msg_mask_table_entry(mask, &range);
		if (err)
			break;
	}
	mutex_unlock(&msg_mask.lock);
	return err;
}

static int diag_create_build_time_mask(void)
{
	int i;
	int err = 0;
	const uint32_t *tbl = NULL;
	uint32_t tbl_size = 0;
	struct diag_msg_mask_t *build_mask = NULL;
	struct diag_ssid_range_t range;

	mutex_lock(&msg_bt_mask.lock);
	build_mask = (struct diag_msg_mask_t *)msg_bt_mask.ptr;
	for (i = 0; i < driver->msg_mask_tbl_count; i++, build_mask++) {
		range.ssid_first = msg_mask_tbl[i].ssid_first;
		range.ssid_last = msg_mask_tbl[i].ssid_last;
		err = diag_create_msg_mask_table_entry(build_mask, &range);
		if (err)
			break;
		switch (build_mask->ssid_first) {
		case MSG_SSID_0:
			tbl = msg_bld_masks_0;
			tbl_size = sizeof(msg_bld_masks_0);
			break;
		case MSG_SSID_1:
			tbl = msg_bld_masks_1;
			tbl_size = sizeof(msg_bld_masks_1);
			break;
		case MSG_SSID_2:
			tbl = msg_bld_masks_2;
			tbl_size = sizeof(msg_bld_masks_2);
			break;
		case MSG_SSID_3:
			tbl = msg_bld_masks_3;
			tbl_size = sizeof(msg_bld_masks_3);
			break;
		case MSG_SSID_4:
			tbl = msg_bld_masks_4;
			tbl_size = sizeof(msg_bld_masks_4);
			break;
		case MSG_SSID_5:
			tbl = msg_bld_masks_5;
			tbl_size = sizeof(msg_bld_masks_5);
			break;
		case MSG_SSID_6:
			tbl = msg_bld_masks_6;
			tbl_size = sizeof(msg_bld_masks_6);
			break;
		case MSG_SSID_7:
			tbl = msg_bld_masks_7;
			tbl_size = sizeof(msg_bld_masks_7);
			break;
		case MSG_SSID_8:
			tbl = msg_bld_masks_8;
			tbl_size = sizeof(msg_bld_masks_8);
			break;
		case MSG_SSID_9:
			tbl = msg_bld_masks_9;
			tbl_size = sizeof(msg_bld_masks_9);
			break;
		case MSG_SSID_10:
			tbl = msg_bld_masks_10;
			tbl_size = sizeof(msg_bld_masks_10);
			break;
		case MSG_SSID_11:
			tbl = msg_bld_masks_11;
			tbl_size = sizeof(msg_bld_masks_11);
			break;
		case MSG_SSID_12:
			tbl = msg_bld_masks_12;
			tbl_size = sizeof(msg_bld_masks_12);
			break;
		case MSG_SSID_13:
			tbl = msg_bld_masks_13;
			tbl_size = sizeof(msg_bld_masks_13);
			break;
		case MSG_SSID_14:
			tbl = msg_bld_masks_14;
			tbl_size = sizeof(msg_bld_masks_14);
			break;
		case MSG_SSID_15:
			tbl = msg_bld_masks_15;
			tbl_size = sizeof(msg_bld_masks_15);
			break;
		case MSG_SSID_16:
			tbl = msg_bld_masks_16;
			tbl_size = sizeof(msg_bld_masks_16);
			break;
		case MSG_SSID_17:
			tbl = msg_bld_masks_17;
			tbl_size = sizeof(msg_bld_masks_17);
			break;
		case MSG_SSID_18:
			tbl = msg_bld_masks_18;
			tbl_size = sizeof(msg_bld_masks_18);
			break;
		case MSG_SSID_19:
			tbl = msg_bld_masks_19;
			tbl_size = sizeof(msg_bld_masks_19);
			break;
		case MSG_SSID_20:
			tbl = msg_bld_masks_20;
			tbl_size = sizeof(msg_bld_masks_20);
			break;
		case MSG_SSID_21:
			tbl = msg_bld_masks_21;
			tbl_size = sizeof(msg_bld_masks_21);
			break;
		case MSG_SSID_22:
			tbl = msg_bld_masks_22;
			tbl_size = sizeof(msg_bld_masks_22);
			break;
		}
		if (!tbl)
			continue;
		if (tbl_size > build_mask->range * sizeof(uint32_t)) {
			pr_warn("diag: In %s, table %d has more ssid than max, ssid_first: %d, ssid_last: %d\n",
				__func__, i, build_mask->ssid_first,
				build_mask->ssid_last);
			tbl_size = build_mask->range * sizeof(uint32_t);
		}
		memcpy(build_mask->ptr, tbl, tbl_size);
	}
	mutex_unlock(&msg_bt_mask.lock);

	return err;
}

static int diag_create_log_mask_table(void)
{
	struct diag_log_mask_t *mask = NULL;
	uint8_t i;
	int err = 0;

	mutex_lock(&log_mask.lock);
	mask = (struct diag_log_mask_t *)(log_mask.ptr);
	for (i = 0; i < MAX_EQUIP_ID; i++, mask++) {
		mask->equip_id = i;
		mask->num_items = LOG_GET_ITEM_NUM(log_code_last_tbl[i]);
		if (LOG_ITEMS_TO_SIZE(mask->num_items) > MAX_ITEMS_PER_EQUIP_ID)
			mask->range = LOG_ITEMS_TO_SIZE(mask->num_items);
		else
			mask->range = MAX_ITEMS_PER_EQUIP_ID;
		mask->ptr = kzalloc(mask->range, GFP_KERNEL);
		if (!mask->ptr) {
			err = -ENOMEM;
			break;
		}
		kmemleak_not_leak(mask->ptr);
	}
	mutex_unlock(&log_mask.lock);
	return err;
}

static int __diag_mask_init(struct diag_mask_info *mask_info, int mask_len,
			    int update_buf_len)
{
	if (!mask_info || mask_len < 0 || update_buf_len < 0)
		return -EINVAL;

	mask_info->status = DIAG_CTRL_MASK_INVALID;
	mask_info->mask_len = mask_len;
	mask_info->update_buf_len = update_buf_len;
	if (mask_len > 0) {
		mask_info->ptr = kzalloc(mask_len, GFP_KERNEL);
		if (!mask_info->ptr)
			return -ENOMEM;
		kmemleak_not_leak(mask_info->ptr);
	}
	if (update_buf_len > 0) {
		mask_info->update_buf = kzalloc(update_buf_len, GFP_KERNEL);
		if (!mask_info->update_buf) {
			kfree(mask_info->ptr);
			return -ENOMEM;
		}
		kmemleak_not_leak(mask_info->update_buf);
	}
	mutex_init(&mask_info->lock);
	return 0;
}

static int diag_msg_mask_init(void)
{
	int err = 0;
	int i;

	err = __diag_mask_init(&msg_mask, MSG_MASK_SIZE, APPS_BUF_SIZE);
	if (err)
		return err;
	err = diag_create_msg_mask_table();
	if (err) {
		pr_err("diag: Unable to create msg masks, err: %d\n", err);
		return err;
	}
	driver->msg_mask = &msg_mask;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		driver->max_ssid_count[i] = 0;

	return 0;
}

static void diag_msg_mask_exit(void)
{
	int i;
	struct diag_msg_mask_t *mask = NULL;

	mask = (struct diag_msg_mask_t *)(msg_mask.ptr);
	if (mask) {
		for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++)
			kfree(mask->ptr);
		kfree(msg_mask.ptr);
	}

	kfree(msg_mask.update_buf);
}

static int diag_build_time_mask_init(void)
{
	int err = 0;

	/* There is no need for update buffer for Build Time masks */
	err = __diag_mask_init(&msg_bt_mask, MSG_MASK_SIZE, 0);
	if (err)
		return err;
	err = diag_create_build_time_mask();
	if (err) {
		pr_err("diag: Unable to create msg build time masks, err: %d\n",
		       err);
		return err;
	}
	driver->build_time_mask = &msg_bt_mask;
	return 0;
}

static void diag_build_time_mask_exit(void)
{
	int i;
	struct diag_msg_mask_t *mask = NULL;

	mask = (struct diag_msg_mask_t *)(msg_bt_mask.ptr);
	if (mask) {
		for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++)
			kfree(mask->ptr);
		kfree(msg_mask.ptr);
	}
}

static int diag_log_mask_init(void)
{
	int err = 0;
	int i;

	err = __diag_mask_init(&log_mask, LOG_MASK_SIZE, APPS_BUF_SIZE);
	if (err)
		return err;
	err = diag_create_log_mask_table();
	if (err)
		return err;
	driver->log_mask = &log_mask;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		driver->num_equip_id[i] = 0;

	return 0;
}

static void diag_log_mask_exit(void)
{
	int i;
	struct diag_log_mask_t *mask = NULL;

	mask = (struct diag_log_mask_t *)(log_mask.ptr);
	if (mask) {
		for (i = 0; i < MAX_EQUIP_ID; i++, mask++)
			kfree(mask->ptr);
		kfree(log_mask.ptr);
	}

	kfree(log_mask.update_buf);
}

static int diag_event_mask_init(void)
{
	int err = 0;
	int i;

	err = __diag_mask_init(&event_mask, EVENT_MASK_SIZE, APPS_BUF_SIZE);
	if (err)
		return err;
	driver->event_mask_size = EVENT_MASK_SIZE;
	driver->last_event_id = APPS_EVENT_LAST_ID;
	driver->event_mask = &event_mask;

	for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++)
		driver->num_event_id[i] = 0;

	return 0;
}

static void diag_event_mask_exit(void)
{
	kfree(event_mask.ptr);
	kfree(event_mask.update_buf);
}

int diag_copy_to_user_msg_mask(char __user *buf, size_t count)
{
	int i;
	int err = 0;
	int len = 0;
	int copy_len = 0;
	int total_len = 0;
	struct diag_msg_mask_userspace_t header;
	struct diag_msg_mask_t *mask = NULL;
	unsigned char *ptr = NULL;

	if (!buf || count == 0)
		return -EINVAL;

	mutex_lock(&msg_mask.lock);
	mask = (struct diag_msg_mask_t *)(msg_mask.ptr);
	for (i = 0; i < driver->msg_mask_tbl_count; i++, mask++) {
		ptr = msg_mask.update_buf;
		len = 0;
		header.ssid_first = mask->ssid_first;
		header.ssid_last = mask->ssid_last;
		header.range = mask->range;
		memcpy(ptr, &header, sizeof(header));
		len += sizeof(header);
		copy_len = (sizeof(uint32_t) * mask->range);
		if ((len + copy_len) > msg_mask.update_buf_len) {
			pr_err("diag: In %s, no space to update msg mask, first: %d, last: %d\n",
			       __func__, mask->ssid_first, mask->ssid_last);
			continue;
		}
		memcpy(ptr + len, mask->ptr, copy_len);
		len += copy_len;
		/* + sizeof(int) to account for data_type already in buf */
		if (total_len + sizeof(int) + len > count) {
			pr_err("diag: In %s, unable to send msg masks to user space, total_len: %d, count: %zu\n",
			       __func__, total_len, count);
			err = -ENOMEM;
			break;
		}
		err = copy_to_user(buf + total_len, (void *)ptr, len);
		if (err) {
			pr_err("diag: In %s Unable to send msg masks to user space clients, err: %d\n",
			       __func__, err);
			break;
		}
		total_len += len;
	}
	mutex_unlock(&msg_mask.lock);

	return err ? err : total_len;
}

int diag_copy_to_user_log_mask(char __user *buf, size_t count)
{
	int i;
	int err = 0;
	int len = 0;
	int copy_len = 0;
	int total_len = 0;
	struct diag_log_mask_userspace_t header;
	struct diag_log_mask_t *mask = NULL;
	unsigned char *ptr = NULL;

	if (!buf || count == 0)
		return -EINVAL;

	mutex_lock(&log_mask.lock);
	mask = (struct diag_log_mask_t *)(log_mask.ptr);
	for (i = 0; i < MAX_EQUIP_ID; i++, mask++) {
		ptr = log_mask.update_buf;
		len = 0;
		header.equip_id = mask->equip_id;
		header.num_items = mask->num_items;
		memcpy(ptr, &header, sizeof(header));
		len += sizeof(header);
		copy_len = LOG_ITEMS_TO_SIZE(header.num_items);
		if ((len + copy_len) > log_mask.update_buf_len) {
			pr_err("diag: In %s, no space to update log mask, equip_id: %d\n",
			       __func__, mask->equip_id);
			continue;
		}
		memcpy(ptr + len, mask->ptr, copy_len);
		len += copy_len;
		/* + sizeof(int) to account for data_type already in buf */
		if (total_len + sizeof(int) + len > count) {
			pr_err("diag: In %s, unable to send log masks to user space, total_len: %d, count: %zu\n",
			       __func__, total_len, count);
			err = -ENOMEM;
			break;
		}
		err = copy_to_user(buf + total_len, (void *)ptr, len);
		if (err) {
			pr_err("diag: In %s Unable to send log masks to user space clients, err: %d\n",
			       __func__, err);
			break;
		}
		total_len += len;
	}
	mutex_unlock(&log_mask.lock);

	return err ? err : total_len;
}

void diag_mask_update_fn(struct work_struct *work)
{
	struct diag_smd_info *smd_info = container_of(work,
						struct diag_smd_info,
						diag_notify_update_smd_work);
	if (!smd_info) {
		pr_err("diag: In %s, smd info is null, cannot update masks for the peripheral\n",
			__func__);
		return;
	}

	diag_send_feature_mask_update(smd_info);
	diag_send_msg_mask_update(smd_info, ALL_SSID, ALL_SSID);
	diag_send_log_mask_update(smd_info, ALL_EQUIP_ID);
	diag_send_event_mask_update(smd_info);

	if (smd_info->notify_context == SMD_EVENT_OPEN) {
		diag_send_diag_mode_update_by_smd(smd_info,
				driver->real_time_mode[DIAG_LOCAL_PROC]);
		diag_send_peripheral_buffering_mode(
				&driver->buffering_mode[smd_info->peripheral]);
	}

	smd_info->notify_context = 0;
}

int diag_process_apps_masks(unsigned char *buf, int len)
{
	int size = 0;
	int sub_cmd = 0;
	int (*hdlr)(unsigned char *src_buf, int src_len,
		    unsigned char *dest_buf, int dest_len) = NULL;

	if (!buf || len <= 0)
		return -EINVAL;

	if (*buf == DIAG_CMD_LOG_CONFIG) {
		sub_cmd = *(int *)(buf + sizeof(int));
		switch (sub_cmd) {
		case DIAG_CMD_OP_LOG_DISABLE:
			hdlr = diag_cmd_disable_log_mask;
			break;
		case DIAG_CMD_OP_GET_LOG_RANGE:
			hdlr = diag_cmd_get_log_range;
			break;
		case DIAG_CMD_OP_SET_LOG_MASK:
			hdlr = diag_cmd_set_log_mask;
			break;
		case DIAG_CMD_OP_GET_LOG_MASK:
			hdlr = diag_cmd_get_log_mask;
			break;
		}
	} else if (*buf == DIAG_CMD_MSG_CONFIG) {
		sub_cmd = *(uint8_t *)(buf + sizeof(uint8_t));
		switch (sub_cmd) {
		case DIAG_CMD_OP_GET_SSID_RANGE:
			hdlr = diag_cmd_get_ssid_range;
			break;
		case DIAG_CMD_OP_GET_BUILD_MASK:
			hdlr = diag_cmd_get_build_mask;
			break;
		case DIAG_CMD_OP_GET_MSG_MASK:
			hdlr = diag_cmd_get_msg_mask;
			break;
		case DIAG_CMD_OP_SET_MSG_MASK:
			hdlr = diag_cmd_set_msg_mask;
			break;
		case DIAG_CMD_OP_SET_ALL_MSG_MASK:
			hdlr = diag_cmd_set_all_msg_mask;
			break;
		}
	} else if (*buf == DIAG_CMD_GET_EVENT_MASK) {
		hdlr = diag_cmd_get_event_mask;
	} else if (*buf == DIAG_CMD_SET_EVENT_MASK) {
		hdlr = diag_cmd_update_event_mask;
	} else if (*buf == DIAG_CMD_EVENT_TOGGLE) {
		hdlr = diag_cmd_toggle_events;
	}

	if (hdlr)
		size = hdlr(buf, len, driver->apps_rsp_buf, APPS_BUF_SIZE);

	return (size > 0) ? size : 0;
}

int diag_masks_init(void)
{
	int err = 0;
	err = diag_msg_mask_init();
	if (err)
		goto fail;

	err = diag_build_time_mask_init();
	if (err)
		goto fail;

	err = diag_log_mask_init();
	if (err)
		goto fail;

	err = diag_event_mask_init();
	if (err)
		goto fail;

	if (driver->buf_feature_mask_update == NULL) {
		driver->buf_feature_mask_update = kzalloc(sizeof(
					struct diag_ctrl_feature_mask) +
					FEATURE_MASK_LEN, GFP_KERNEL);
		if (driver->buf_feature_mask_update == NULL)
			goto fail;
		kmemleak_not_leak(driver->buf_feature_mask_update);
	}

	return 0;
fail:
	pr_err("diag: Could not initialize diag mask buffers\n");
	diag_masks_exit();
	return -ENOMEM;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

void diag_masks_exit(void)
{
<<<<<<< HEAD
	kfree(driver->event_mask);
	kfree(driver->log_mask);
	kfree(driver->msg_mask);
	kfree(driver->msg_masks);
	kfree(driver->log_masks);
	kfree(driver->event_masks);
	kfree(driver->feature_mask);
=======
	diag_msg_mask_exit();
	diag_build_time_mask_exit();
	diag_log_mask_exit();
	diag_event_mask_exit();
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	kfree(driver->buf_feature_mask_update);
}
