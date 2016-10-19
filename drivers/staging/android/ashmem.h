<<<<<<< HEAD
/*
 * include/linux/ashmem.h
 *
 * Copyright 2008 Google Inc.
 * Author: Robert Love
 *
 * This file is dual licensed.  It may be redistributed and/or modified
 * under the terms of the Apache 2.0 License OR version 2 of the GNU
 * General Public License.
 */

#ifndef _LINUX_ASHMEM_H
#define _LINUX_ASHMEM_H

#include <linux/limits.h>
#include <linux/ioctl.h>
#include <linux/compat.h>

#define ASHMEM_NAME_LEN		256

#define ASHMEM_NAME_DEF		"dev/ashmem"

/* Return values from ASHMEM_PIN: Was the mapping purged while unpinned? */
#define ASHMEM_NOT_PURGED	0
#define ASHMEM_WAS_PURGED	1

/* Return values from ASHMEM_GET_PIN_STATUS: Is the mapping pinned? */
#define ASHMEM_IS_UNPINNED	0
#define ASHMEM_IS_PINNED	1

struct ashmem_pin {
	__u32 offset;	/* offset into region, in bytes, page-aligned */
	__u32 len;	/* length forward from offset, in bytes, page-aligned */
};

#define __ASHMEMIOC		0x77

#define ASHMEM_SET_NAME		_IOW(__ASHMEMIOC, 1, char[ASHMEM_NAME_LEN])
#define ASHMEM_GET_NAME		_IOR(__ASHMEMIOC, 2, char[ASHMEM_NAME_LEN])
#define ASHMEM_SET_SIZE		_IOW(__ASHMEMIOC, 3, size_t)
#define ASHMEM_GET_SIZE		_IO(__ASHMEMIOC, 4)
#define ASHMEM_SET_PROT_MASK	_IOW(__ASHMEMIOC, 5, unsigned long)
#define ASHMEM_GET_PROT_MASK	_IO(__ASHMEMIOC, 6)
#define ASHMEM_PIN		_IOW(__ASHMEMIOC, 7, struct ashmem_pin)
#define ASHMEM_UNPIN		_IOW(__ASHMEMIOC, 8, struct ashmem_pin)
#define ASHMEM_GET_PIN_STATUS	_IO(__ASHMEMIOC, 9)
#define ASHMEM_PURGE_ALL_CACHES	_IO(__ASHMEMIOC, 10)

=======
/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ANDROID_ASHMEM_H
#define _ANDROID_ASHMEM_H

#include <linux/compat.h>

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
#define COMPAT_ASHMEM_SET_SIZE		_IOW(__ASHMEMIOC, 3, compat_size_t)
#define COMPAT_ASHMEM_SET_PROT_MASK	_IOW(__ASHMEMIOC, 5, unsigned int)
#endif

<<<<<<< HEAD
#endif	/* _LINUX_ASHMEM_H */
=======
#endif
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
