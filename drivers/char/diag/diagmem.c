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
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mempool.h>
<<<<<<< HEAD
#include <linux/mutex.h>
#include <asm/atomic.h>
#include "diagchar.h"
#include "diagfwd_bridge.h"
#include "diagfwd_hsic.h"

mempool_t *diag_pools_array[NUM_MEMORY_POOLS];

void *diagmem_alloc(struct diagchar_dev *driver, int size, int pool_type)
{
	void *buf = NULL;
	unsigned long flags;
	int index;

	spin_lock_irqsave(&driver->diag_mem_lock, flags);
	index = 0;
	if (pool_type == POOL_TYPE_COPY) {
		if (driver->diagpool) {
			if ((driver->count < driver->poolsize) &&
				(size <= driver->itemsize)) {
				atomic_add(1, (atomic_t *)&driver->count);
				buf = mempool_alloc(driver->diagpool,
								 GFP_ATOMIC);
			}
		}
	} else if (pool_type == POOL_TYPE_HDLC) {
		if (driver->diag_hdlc_pool) {
			if ((driver->count_hdlc_pool < driver->poolsize_hdlc) &&
				(size <= driver->itemsize_hdlc)) {
				atomic_add(1,
					 (atomic_t *)&driver->count_hdlc_pool);
				buf = mempool_alloc(driver->diag_hdlc_pool,
								 GFP_ATOMIC);
			}
		}
	} else if (pool_type == POOL_TYPE_USER) {
		if (driver->diag_user_pool) {
			if ((driver->count_user_pool < driver->poolsize_user) &&
				(size <= driver->itemsize_user)) {
				atomic_add(1,
					(atomic_t *)&driver->count_user_pool);
				buf = mempool_alloc(driver->diag_user_pool,
					GFP_ATOMIC);
			}
		}
	} else if (pool_type == POOL_TYPE_WRITE_STRUCT) {
		if (driver->diag_write_struct_pool) {
			if ((driver->count_write_struct_pool <
			     driver->poolsize_write_struct) &&
			     (size <= driver->itemsize_write_struct)) {
				atomic_add(1,
				 (atomic_t *)&driver->count_write_struct_pool);
				buf = mempool_alloc(
				driver->diag_write_struct_pool, GFP_ATOMIC);
			}
		}
	} else if (pool_type == POOL_TYPE_DCI) {
		if (driver->diag_dci_pool) {
			if ((driver->count_dci_pool < driver->poolsize_dci) &&
				(size <= driver->itemsize_dci)) {
				atomic_add(1,
					(atomic_t *)&driver->count_dci_pool);
				buf = mempool_alloc(driver->diag_dci_pool,
								GFP_ATOMIC);
			}
		}
#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
	} else if (pool_type == POOL_TYPE_HSIC ||
				pool_type == POOL_TYPE_HSIC_2) {
		index = pool_type - POOL_TYPE_HSIC;
		if (diag_hsic[index].diag_hsic_pool) {
			if ((diag_hsic[index].count_hsic_pool <
			     diag_hsic[index].poolsize_hsic) &&
			     (size <= diag_hsic[index].itemsize_hsic)) {
				atomic_add(1, (atomic_t *)
					&diag_hsic[index].count_hsic_pool);
				buf = mempool_alloc(
					diag_hsic[index].diag_hsic_pool,
					GFP_ATOMIC);
			}
		}
	} else if (pool_type == POOL_TYPE_HSIC_WRITE ||
					pool_type == POOL_TYPE_HSIC_2_WRITE) {
		index = pool_type - POOL_TYPE_HSIC_WRITE;
		if (diag_hsic[index].diag_hsic_write_pool) {
			if (diag_hsic[index].count_hsic_write_pool <
			    diag_hsic[index].poolsize_hsic_write &&
			    (size <= diag_hsic[index].itemsize_hsic_write)) {
				atomic_add(1, (atomic_t *)
					&diag_hsic[index].
					count_hsic_write_pool);
				buf = mempool_alloc(
					diag_hsic[index].diag_hsic_write_pool,
					GFP_ATOMIC);
			}
		}
#endif
	}
	spin_unlock_irqrestore(&driver->diag_mem_lock, flags);
	return buf;
}

void diagmem_exit(struct diagchar_dev *driver, int pool_type)
{
	int index;
	unsigned long flags;
	index = 0;

	spin_lock_irqsave(&driver->diag_mem_lock, flags);
	if (driver->diagpool) {
		if (driver->count == 0 && driver->ref_count == 0) {
			mempool_destroy(driver->diagpool);
			driver->diagpool = NULL;
		} else if (driver->ref_count == 0 && pool_type ==
							POOL_TYPE_ALL) {
			pr_err("diag: Unable to destroy COPY mempool");
		}
	}

	if (driver->diag_hdlc_pool) {
		if (driver->count_hdlc_pool == 0 && driver->ref_count == 0) {
			mempool_destroy(driver->diag_hdlc_pool);
			driver->diag_hdlc_pool = NULL;
		} else if (driver->ref_count == 0 && pool_type ==
							POOL_TYPE_ALL) {
			pr_err("diag: Unable to destroy HDLC mempool");
		}
	}

	if (driver->diag_user_pool) {
		if (driver->count_user_pool == 0 && driver->ref_count == 0) {
			mempool_destroy(driver->diag_user_pool);
			driver->diag_user_pool = NULL;
		} else if (driver->ref_count == 0 && pool_type ==
							POOL_TYPE_ALL) {
			pr_err("diag: Unable to destroy USER mempool");
		}
	}

	if (driver->diag_write_struct_pool) {
		/* Free up struct pool ONLY if there are no outstanding
		transactions(aggregation buffer) with USB */
		if (driver->count_write_struct_pool == 0 &&
		 driver->count_hdlc_pool == 0 && driver->ref_count == 0) {
			mempool_destroy(driver->diag_write_struct_pool);
			driver->diag_write_struct_pool = NULL;
		} else if (driver->ref_count == 0 && pool_type ==
							POOL_TYPE_ALL) {
			pr_err("diag: Unable to destroy STRUCT mempool");
		}
	}

	if (driver->diag_dci_pool) {
		if (driver->count_dci_pool == 0 && driver->ref_count == 0) {
			mempool_destroy(driver->diag_dci_pool);
			driver->diag_dci_pool = NULL;
		} else if (driver->ref_count == 0 && pool_type ==
							POOL_TYPE_ALL) {
				pr_err("diag: Unable to destroy DCI mempool");
		}
	}
#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
	for (index = 0; index < MAX_HSIC_CH; index++) {
		if (diag_hsic[index].diag_hsic_pool &&
				(diag_hsic[index].hsic_inited == 0)) {
			if (diag_hsic[index].count_hsic_pool == 0) {
				mempool_destroy(
					diag_hsic[index].diag_hsic_pool);
				diag_hsic[index].diag_hsic_pool = NULL;
			} else if (pool_type == POOL_TYPE_ALL)
				pr_err("Unable to destroy HDLC mempool for ch %d"
								, index);
		}

		if (diag_hsic[index].diag_hsic_write_pool &&
					(diag_hsic[index].hsic_inited == 0)) {
			/*
			 * Free up struct pool ONLY if there are no outstanding
			 * transactions(aggregation buffer) with USB
			 */
			if (diag_hsic[index].count_hsic_write_pool == 0 &&
				diag_hsic[index].count_hsic_pool == 0) {
				mempool_destroy(
					diag_hsic[index].diag_hsic_write_pool);
				diag_hsic[index].diag_hsic_write_pool = NULL;
			} else if (pool_type == POOL_TYPE_ALL)
				pr_err("Unable to destroy HSIC USB struct mempool for ch %d"
								, index);
		}
	}
#endif
	spin_unlock_irqrestore(&driver->diag_mem_lock, flags);
=======
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/kmemleak.h>
#include <linux/ratelimit.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/kmemleak.h>

#include "diagchar.h"
#include "diagmem.h"

struct diag_mempool_t diag_mempools[NUM_MEMORY_POOLS] = {
	{
		.id = POOL_TYPE_COPY,
		.name = "POOL_COPY",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_HDLC,
		.name = "POOL_HDLC",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_USER,
		.name = "POOL_USER",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MUX_APPS,
		.name = "POOL_MUX_APPS",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_DCI,
		.name = "POOL_DCI",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
	{
		.id = POOL_TYPE_MDM,
		.name = "POOL_MDM",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM2,
		.name = "POOL_MDM2",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM_DCI,
		.name = "POOL_MDM_DCI",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM2_DCI,
		.name = "POOL_MDM2_DCI",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM_MUX,
		.name = "POOL_MDM_MUX",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM2_MUX,
		.name = "POOL_MDM2_MUX",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM_DCI_WRITE,
		.name = "POOL_MDM_DCI_WRITE",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_MDM2_DCI_WRITE,
		.name = "POOL_MDM2_DCI_WRITE",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	},
	{
		.id = POOL_TYPE_QSC_MUX,
		.name = "POOL_QSC_MUX",
		.pool = NULL,
		.itemsize = 0,
		.poolsize = 0,
		.count = 0
	}
#endif
};

void diagmem_setsize(int pool_idx, int itemsize, int poolsize)
{
	if (pool_idx < 0 || pool_idx >= NUM_MEMORY_POOLS) {
		pr_err("diag: Invalid pool index %d in %s\n", pool_idx,
		       __func__);
		return;
	}

	diag_mempools[pool_idx].itemsize = itemsize;
	diag_mempools[pool_idx].poolsize = poolsize;
	pr_debug("diag: Mempool %s sizes: itemsize %d poolsize %d\n",
		 diag_mempools[pool_idx].name, diag_mempools[pool_idx].itemsize,
		 diag_mempools[pool_idx].poolsize);
}

void *diagmem_alloc(struct diagchar_dev *driver, int size, int pool_type)
{
	void *buf = NULL;
	int i = 0;
	unsigned long flags;
	struct diag_mempool_t *mempool = NULL;

	if (!driver)
		return NULL;

	for (i = 0; i < NUM_MEMORY_POOLS; i++) {
		mempool = &diag_mempools[i];
		if (pool_type != mempool->id)
			continue;
		if (!mempool->pool) {
			pr_err_ratelimited("diag: %s mempool is not initialized yet\n",
					   mempool->name);
			break;
		}
		if (size == 0 || size > mempool->itemsize) {
			pr_err_ratelimited("diag: cannot alloc from mempool %s, invalid size: %d\n",
					   mempool->name, size);
			break;
		}
		spin_lock_irqsave(&mempool->lock, flags);
		if (mempool->count < mempool->poolsize) {
			atomic_add(1, (atomic_t *)&mempool->count);
			buf = mempool_alloc(mempool->pool, GFP_ATOMIC);
			kmemleak_not_leak(buf);
		}
		spin_unlock_irqrestore(&mempool->lock, flags);
		if (!buf) {
			pr_debug_ratelimited("diag: Unable to allocate buffer from memory pool %s, size: %d/%d count: %d/%d\n",
					     mempool->name,
					     size, mempool->itemsize,
					     mempool->count,
					     mempool->poolsize);
		}
		break;
	}

	return buf;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

void diagmem_free(struct diagchar_dev *driver, void *buf, int pool_type)
{
<<<<<<< HEAD
	int index;
	unsigned long flags;

	if (!buf)
		return;

	spin_lock_irqsave(&driver->diag_mem_lock, flags);
	index = 0;
	if (pool_type == POOL_TYPE_COPY) {
		if (driver->diagpool != NULL && driver->count > 0) {
			mempool_free(buf, driver->diagpool);
			atomic_add(-1, (atomic_t *)&driver->count);
		} else
			pr_err("diag: Attempt to free up DIAG driver mempool memory which is already free %d",
							driver->count);
	} else if (pool_type == POOL_TYPE_HDLC) {
		if (driver->diag_hdlc_pool != NULL &&
			 driver->count_hdlc_pool > 0) {
			mempool_free(buf, driver->diag_hdlc_pool);
			atomic_add(-1, (atomic_t *)&driver->count_hdlc_pool);
		} else
			pr_err("diag: Attempt to free up DIAG driver HDLC mempool which is already free %d ",
						driver->count_hdlc_pool);
	} else if (pool_type == POOL_TYPE_USER) {
		if (driver->diag_user_pool != NULL &&
			driver->count_user_pool > 0) {
			mempool_free(buf, driver->diag_user_pool);
			atomic_add(-1, (atomic_t *)&driver->count_user_pool);
		} else {
			pr_err("diag: Attempt to free up DIAG driver USER mempool which is already free %d ",
						driver->count_user_pool);
		}
	} else if (pool_type == POOL_TYPE_WRITE_STRUCT) {
		if (driver->diag_write_struct_pool != NULL &&
			 driver->count_write_struct_pool > 0) {
			mempool_free(buf, driver->diag_write_struct_pool);
			atomic_add(-1,
				 (atomic_t *)&driver->count_write_struct_pool);
		} else
			pr_err("diag: Attempt to free up DIAG driver USB structure mempool which is already free %d ",
					driver->count_write_struct_pool);
	} else if (pool_type == POOL_TYPE_DCI) {
		if (driver->diag_dci_pool != NULL &&
			driver->count_dci_pool > 0) {
				mempool_free(buf, driver->diag_dci_pool);
				atomic_add(-1,
					(atomic_t *)&driver->count_dci_pool);
		} else
			pr_err("diag: Attempt to free up DIAG driver DCI mempool which is already free %d ",
					driver->count_dci_pool);
#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
	} else if (pool_type == POOL_TYPE_HSIC ||
				pool_type == POOL_TYPE_HSIC_2) {
		index = pool_type - POOL_TYPE_HSIC;
		if (diag_hsic[index].diag_hsic_pool != NULL &&
			diag_hsic[index].count_hsic_pool > 0) {
			mempool_free(buf, diag_hsic[index].diag_hsic_pool);
			atomic_add(-1, (atomic_t *)
				   &diag_hsic[index].count_hsic_pool);
		} else
			pr_err("diag: Attempt to free up DIAG driver HSIC mempool which is already free %d, ch = %d",
				diag_hsic[index].count_hsic_pool, index);
	} else if (pool_type == POOL_TYPE_HSIC_WRITE ||
				pool_type == POOL_TYPE_HSIC_2_WRITE) {
		index = pool_type - POOL_TYPE_HSIC_WRITE;
		if (diag_hsic[index].diag_hsic_write_pool != NULL &&
			diag_hsic[index].count_hsic_write_pool > 0) {
			mempool_free(buf,
					diag_hsic[index].diag_hsic_write_pool);
			atomic_add(-1, (atomic_t *)
				&diag_hsic[index].count_hsic_write_pool);
		} else
			pr_err("diag: Attempt to free up DIAG driver HSIC USB structure mempool which is already free %d, ch = %d",
				driver->count_write_struct_pool, index);
#endif
	} else {
		pr_err("diag: In %s, unknown pool type: %d\n",
			__func__, pool_type);

	}
	spin_unlock_irqrestore(&driver->diag_mem_lock, flags);
	diagmem_exit(driver, pool_type);
}

void diagmem_init(struct diagchar_dev *driver)
{
	spin_lock_init(&driver->diag_mem_lock);

	if (driver->count == 0) {
		driver->diagpool = mempool_create_kmalloc_pool(
					driver->poolsize, driver->itemsize);
		diag_pools_array[POOL_COPY_IDX] = driver->diagpool;
	}

	if (driver->count_hdlc_pool == 0) {
		driver->diag_hdlc_pool = mempool_create_kmalloc_pool(
				driver->poolsize_hdlc, driver->itemsize_hdlc);
		diag_pools_array[POOL_HDLC_IDX] = driver->diag_hdlc_pool;
	}

	if (driver->count_user_pool == 0) {
		driver->diag_user_pool = mempool_create_kmalloc_pool(
				driver->poolsize_user, driver->itemsize_user);
		diag_pools_array[POOL_USER_IDX] = driver->diag_user_pool;
	}

	if (driver->count_write_struct_pool == 0) {
		driver->diag_write_struct_pool = mempool_create_kmalloc_pool(
		driver->poolsize_write_struct, driver->itemsize_write_struct);
		diag_pools_array[POOL_WRITE_STRUCT_IDX] =
						driver->diag_write_struct_pool;
	}

	if (driver->count_dci_pool == 0) {
		driver->diag_dci_pool = mempool_create_kmalloc_pool(
			driver->poolsize_dci, driver->itemsize_dci);
		diag_pools_array[POOL_DCI_IDX] = driver->diag_dci_pool;
	}

	if (!driver->diagpool)
		pr_err("diag: Cannot allocate diag mempool\n");

	if (!driver->diag_hdlc_pool)
		pr_err("diag: Cannot allocate diag HDLC mempool\n");

	if (!driver->diag_user_pool)
		pr_err("diag: Cannot allocate diag USER mempool\n");

	if (!driver->diag_write_struct_pool)
		pr_err("diag: Cannot allocate diag USB struct mempool\n");

	if (!driver->diag_dci_pool)
		pr_err("diag: Cannot allocate diag DCI mempool\n");

}

#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
void diagmem_hsic_init(int index)
{
	if (index < 0 || index >= MAX_HSIC_CH) {
		pr_err("diag: Invalid hsic index in %s\n", __func__);
		return;
	}

	if (diag_hsic[index].count_hsic_pool == 0) {
		diag_hsic[index].diag_hsic_pool = mempool_create_kmalloc_pool(
					diag_hsic[index].poolsize_hsic,
					diag_hsic[index].itemsize_hsic);
		diag_pools_array[POOL_HSIC_IDX + index] =
						diag_hsic[index].diag_hsic_pool;
	}

	if (diag_hsic[index].count_hsic_write_pool == 0) {
		diag_hsic[index].diag_hsic_write_pool =
				mempool_create_kmalloc_pool(
					diag_hsic[index].poolsize_hsic_write,
					diag_hsic[index].itemsize_hsic_write);
		diag_pools_array[POOL_HSIC_WRITE_IDX + index] =
					diag_hsic[index].diag_hsic_write_pool;
	}

	if (!diag_hsic[index].diag_hsic_pool)
		pr_err("Cannot allocate diag HSIC mempool for ch %d\n", index);

	if (!diag_hsic[index].diag_hsic_write_pool)
		pr_err("Cannot allocate diag HSIC struct mempool for ch %d\n",
									index);

}
#endif
=======
	int i = 0;
	unsigned long flags;
	struct diag_mempool_t *mempool = NULL;

	if (!driver || !buf)
		return;

	for (i = 0; i < NUM_MEMORY_POOLS; i++) {
		mempool = &diag_mempools[i];
		if (pool_type != mempool->id)
			continue;
		if (!mempool->pool) {
			pr_err_ratelimited("diag: %s mempool is not initialized yet\n",
					   mempool->name);
			break;
		}
		spin_lock_irqsave(&mempool->lock, flags);
		if (mempool->count > 0) {
			mempool_free(buf, mempool->pool);
			atomic_add(-1, (atomic_t *)&mempool->count);
		} else {
			pr_err_ratelimited("diag: Attempting to free items from %s mempool which is already empty\n",
					   mempool->name);
		}
		spin_unlock_irqrestore(&mempool->lock, flags);
		break;
	}
}

void diagmem_init(struct diagchar_dev *driver, int index)
{
	struct diag_mempool_t *mempool = NULL;
	if (!driver)
		return;

	if (index < 0 || index >= NUM_MEMORY_POOLS) {
		pr_err("diag: In %s, Invalid index %d\n", __func__, index);
		return;
	}

	mempool = &diag_mempools[index];
	if (mempool->pool) {
		pr_debug("diag: mempool %s is already initialized\n",
			 mempool->name);
		return;
	}
	if (mempool->itemsize <= 0 || mempool->poolsize <= 0) {
		pr_err("diag: Unable to initialize %s mempool, itemsize: %d poolsize: %d\n",
		       mempool->name, mempool->itemsize,
		       mempool->poolsize);
		return;
	}

	mempool->pool = mempool_create_kmalloc_pool(mempool->poolsize,
						    mempool->itemsize);
	if (!mempool->pool)
		pr_err("diag: cannot allocate %s mempool\n", mempool->name);
	else
		kmemleak_not_leak(mempool->pool);

	spin_lock_init(&mempool->lock);
}

void diagmem_exit(struct diagchar_dev *driver, int index)
{
	unsigned long flags;
	struct diag_mempool_t *mempool = NULL;

	if (!driver)
		return;

	if (index < 0 || index >= NUM_MEMORY_POOLS) {
		pr_err("diag: In %s, Invalid index %d\n", __func__, index);
		return;
	}

	mempool = &diag_mempools[index];
	spin_lock_irqsave(&mempool->lock, flags);
	if (mempool->count == 0) {
		mempool_destroy(mempool->pool);
		mempool->pool = NULL;
	} else {
		pr_err("diag: Unable to destory %s pool, count: %d\n",
		       mempool->name, mempool->count);
	}
	spin_unlock_irqrestore(&mempool->lock, flags);
}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

