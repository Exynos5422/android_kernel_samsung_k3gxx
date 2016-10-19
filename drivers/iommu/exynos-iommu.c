<<<<<<< HEAD
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/memblock.h>
#include <linux/export.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/device.h>
#include <linux/clk-private.h>
#include <linux/pm_domain.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
=======
/* linux/drivers/iommu/exynos_iommu.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef CONFIG_EXYNOS_IOMMU_DEBUG
#define DEBUG
#endif

#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/iommu.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/memblock.h>
#include <linux/export.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#include <asm/cacheflush.h>
#include <asm/pgtable.h>

<<<<<<< HEAD
#if defined(CONFIG_SOC_EXYNOS5430)
#include <mach/regs-clock.h>
#endif

#include "exynos-iommu.h"

#define MAX_NUM_PPC	4

const char *ppc_event_name[] = {
	"TOTAL",
	"L1TLB MISS",
	"L2TLB MISS",
	"FLPD CACHE MISS",
	"PB LOOK-UP",
	"PB MISS",
	"BLOCK NUM BY PREFETCHING",
	"BLOCK CYCLE BY PREFETCHING",
	"TLB MISS",
	"FLPD MISS ON PREFETCHING",
};

static int iova_from_sent(sysmmu_pte_t *base, sysmmu_pte_t *sent)
{
	return ((unsigned long)sent - (unsigned long)base) *
					(SECT_SIZE / sizeof(sysmmu_pte_t));
}

struct sysmmu_list_data {
	struct device *sysmmu;
	struct list_head node; /* entry of exynos_iommu_owner.mmu_list */
};

#define has_sysmmu(dev)		(dev->archdata.iommu != NULL)
#define for_each_sysmmu_list(dev, sysmmu_list)			\
	list_for_each_entry(sysmmu_list,				\
		&((struct exynos_iommu_owner *)dev->archdata.iommu)->mmu_list,\
		node)

static LIST_HEAD(sysmmu_drvdata_list);
static LIST_HEAD(sysmmu_owner_list);

static struct kmem_cache *lv2table_kmem_cache;
static phys_addr_t fault_page;
unsigned long *zero_lv2_table;
static struct dentry *exynos_sysmmu_debugfs_root;

static inline void pgtable_flush(void *vastart, void *vaend)
{
	dmac_flush_range(vastart, vaend);
	outer_flush_range(virt_to_phys(vastart),
				virt_to_phys(vaend));
}

void sysmmu_tlb_invalidate_flpdcache(struct device *dev, dma_addr_t iova)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		unsigned long flags;
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock_irqsave(&drvdata->lock, flags);
		if (is_sysmmu_active(drvdata) && drvdata->runtime_active) {
			TRACE_LOG_DEV(drvdata->sysmmu,
				"FLPD invalidation @ %#x\n", iova);
			__master_clk_enable(drvdata);
			__sysmmu_tlb_invalidate_flpdcache(
					drvdata->sfrbase, iova);
			SYSMMU_EVENT_LOG_FLPD_FLUSH(
					SYSMMU_DRVDATA_TO_LOG(drvdata), iova);
			__master_clk_disable(drvdata);
		} else {
			TRACE_LOG_DEV(drvdata->sysmmu,
				"Skip FLPD invalidation @ %#x\n", iova);
		}
		spin_unlock_irqrestore(&drvdata->lock, flags);
	}
}

static void sysmmu_tlb_invalidate_entry(struct device *dev, dma_addr_t iova,
					bool force)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		unsigned long flags;
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		if (!force && !(drvdata->prop & SYSMMU_PROP_NONBLOCK_TLBINV))
			continue;

		spin_lock_irqsave(&drvdata->lock, flags);
		if (is_sysmmu_active(drvdata) && drvdata->runtime_active) {
			TRACE_LOG_DEV(drvdata->sysmmu,
				"TLB invalidation @ %#x\n", iova);
			__master_clk_enable(drvdata);
			__sysmmu_tlb_invalidate_entry(drvdata->sfrbase, iova);
			SYSMMU_EVENT_LOG_TLB_INV_VPN(
					SYSMMU_DRVDATA_TO_LOG(drvdata), iova);
			__master_clk_disable(drvdata);
		} else {
			TRACE_LOG_DEV(drvdata->sysmmu,
				"Skip TLB invalidation @ %#x\n", iova);
		}
		spin_unlock_irqrestore(&drvdata->lock, flags);
	}
}

void exynos_sysmmu_tlb_invalidate(struct device *dev, dma_addr_t start,
				  size_t size)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		unsigned long flags;
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		if (!!(drvdata->prop & SYSMMU_PROP_NONBLOCK_TLBINV))
			continue;

		spin_lock_irqsave(&drvdata->lock, flags);
		if (!is_sysmmu_active(drvdata) ||
				!drvdata->runtime_active) {
			spin_unlock_irqrestore(&drvdata->lock, flags);
			TRACE_LOG_DEV(drvdata->sysmmu,
				"Skip TLB invalidation %#x@%#x\n", size, start);
			continue;
		}

		TRACE_LOG_DEV(drvdata->sysmmu,
				"TLB invalidation %#x@%#x\n", size, start);

		__master_clk_enable(drvdata);

		__sysmmu_tlb_invalidate(drvdata, start, size);

		__master_clk_disable(drvdata);

		spin_unlock_irqrestore(&drvdata->lock, flags);
	}
}

static inline void __sysmmu_disable_nocount(struct sysmmu_drvdata *drvdata)
{
	int disable = (drvdata->prop & SYSMMU_PROP_STOP_BLOCK) ?
					CTRL_BLOCK_DISABLE : CTRL_DISABLE;

#if defined(CONFIG_SOC_EXYNOS5430)
	if (!strcmp(dev_name(drvdata->sysmmu), "15200000.sysmmu")) {
		if (!(__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC0_SECURE_SMMU_MFC) & 0x1) ||
			!(__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC0_SECURE_SMMU_MFC) & 0x1)) {
			pr_err("MFC0_0 SYSMMU clock is disabled ACLK: [%#x], PCLK[%#x]\n",
				__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC0_SECURE_SMMU_MFC),
				__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC0_SECURE_SMMU_MFC));
			BUG();
		}
	} else if (!strcmp(dev_name(drvdata->sysmmu), "15210000.sysmmu")) {
		if (!(__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC0_SECURE_SMMU_MFC) & 0x2) ||
			!(__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC0_SECURE_SMMU_MFC) & 0x2)) {
			pr_err("MFC0_1 SYSMMU clock is disabled ACLK: [%#x], PCLK[%#x]\n",
				__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC0_SECURE_SMMU_MFC),
				__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC0_SECURE_SMMU_MFC));
			BUG();
		}
	} else if (!strcmp(dev_name(drvdata->sysmmu), "15300000.sysmmu")) {
		if (!(__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC1_SECURE_SMMU_MFC) & 0x1) ||
			!(__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC1_SECURE_SMMU_MFC) & 0x1)) {
			pr_err("MFC1_0 SYSMMU clock is disabled ACLK: [%#x], PCLK[%#x]\n",
				__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC1_SECURE_SMMU_MFC),
				__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC1_SECURE_SMMU_MFC));
			BUG();
		}
	} else if (!strcmp(dev_name(drvdata->sysmmu), "15310000.sysmmu")) {
		if (!(__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC1_SECURE_SMMU_MFC) & 0x2) ||
			!(__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC1_SECURE_SMMU_MFC) & 0x2)) {
			pr_err("MFC1_1 SYSMMU clock is disabled ACLK: [%#x], PCLK[%#x]\n",
				__raw_readl(EXYNOS5430_ENABLE_ACLK_MFC1_SECURE_SMMU_MFC),
				__raw_readl(EXYNOS5430_ENABLE_PCLK_MFC1_SECURE_SMMU_MFC));
			BUG();
		}
	}
#endif

	__raw_sysmmu_disable(drvdata->sfrbase, disable);

	__sysmmu_clk_disable(drvdata);
	if (IS_ENABLED(CONFIG_EXYNOS_IOMMU_NO_MASTER_CLKGATE))
		__master_clk_disable(drvdata);

	SYSMMU_EVENT_LOG_DISABLE(SYSMMU_DRVDATA_TO_LOG(drvdata));

	TRACE_LOG("%s(%s)\n", __func__, dev_name(drvdata->sysmmu));
}

static bool __sysmmu_disable(struct sysmmu_drvdata *drvdata)
{
	bool disabled;
	unsigned long flags;

	spin_lock_irqsave(&drvdata->lock, flags);

	disabled = set_sysmmu_inactive(drvdata);

	if (disabled) {
		drvdata->pgtable = 0;
		drvdata->domain = NULL;

		if (drvdata->runtime_active) {
			__master_clk_enable(drvdata);
			__sysmmu_disable_nocount(drvdata);
			__master_clk_disable(drvdata);
		}

		TRACE_LOG_DEV(drvdata->sysmmu, "Disabled\n");
	} else  {
		TRACE_LOG_DEV(drvdata->sysmmu, "%d times left to disable\n",
					drvdata->activations);
	}

	spin_unlock_irqrestore(&drvdata->lock, flags);

	return disabled;
}

static void __sysmmu_enable_nocount(struct sysmmu_drvdata *drvdata)
{
	if (IS_ENABLED(CONFIG_EXYNOS_IOMMU_NO_MASTER_CLKGATE))
		__master_clk_enable(drvdata);

	__sysmmu_clk_enable(drvdata);

	__sysmmu_init_config(drvdata);

	__sysmmu_set_ptbase(drvdata->sfrbase, drvdata->pgtable / PAGE_SIZE);

	__raw_sysmmu_enable(drvdata->sfrbase);

	SYSMMU_EVENT_LOG_ENABLE(SYSMMU_DRVDATA_TO_LOG(drvdata));

	TRACE_LOG_DEV(drvdata->sysmmu, "Really enabled\n");
}

static int __sysmmu_enable(struct sysmmu_drvdata *drvdata,
			phys_addr_t pgtable, struct iommu_domain *domain)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&drvdata->lock, flags);
	if (set_sysmmu_active(drvdata)) {
		drvdata->pgtable = pgtable;
		drvdata->domain = domain;

		if (drvdata->runtime_active) {
			__master_clk_enable(drvdata);
			__sysmmu_enable_nocount(drvdata);
			__master_clk_disable(drvdata);
		}

		TRACE_LOG_DEV(drvdata->sysmmu, "Enabled\n");
	} else {
		ret = (pgtable == drvdata->pgtable) ? 1 : -EBUSY;

		TRACE_LOG_DEV(drvdata->sysmmu, "Already enabled (%d)\n", ret);
	}

	if (WARN_ON(ret < 0))
		set_sysmmu_inactive(drvdata); /* decrement count */

	spin_unlock_irqrestore(&drvdata->lock, flags);

	return ret;
}

/* __exynos_sysmmu_enable: Enables System MMU
 *
 * returns -error if an error occurred and System MMU is not enabled,
 * 0 if the System MMU has been just enabled and 1 if System MMU was already
 * enabled before.
 */
static int __exynos_sysmmu_enable(struct device *dev, phys_addr_t pgtable,
				struct iommu_domain *domain)
{
	int ret = 0;
	unsigned long flags;
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;

	BUG_ON(!has_sysmmu(dev));

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);
		drvdata->master = dev;
		ret = __sysmmu_enable(drvdata, pgtable, domain);
		if (ret < 0) {
			struct sysmmu_list_data *iter;
			for_each_sysmmu_list(dev, iter) {
				if (iter == list)
					break;
				__sysmmu_disable(dev_get_drvdata(iter->sysmmu));
				drvdata->master = NULL;
			}
			break;
		}
	}

	spin_unlock_irqrestore(&owner->lock, flags);

	return ret;
}

int exynos_sysmmu_enable(struct device *dev, unsigned long pgtable)
{
	int ret;

	BUG_ON(!memblock_is_memory(pgtable));

	ret = __exynos_sysmmu_enable(dev, pgtable, NULL);

	return ret;
}

bool exynos_sysmmu_disable(struct device *dev)
{
	unsigned long flags;
	bool disabled = true;
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;

	BUG_ON(!has_sysmmu(dev));

	spin_lock_irqsave(&owner->lock, flags);

	/* Every call to __sysmmu_disable() must return same result */
	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);
		disabled = __sysmmu_disable(drvdata);
		if (disabled)
			drvdata->master = NULL;
	}

	spin_unlock_irqrestore(&owner->lock, flags);

	return disabled;
}

#ifdef CONFIG_EXYNOS_IOMMU_RECOVER_FAULT_HANDLER
int recover_fault_handler (struct iommu_domain *domain,
				struct device *dev, unsigned long fault_addr,
				int itype, void *reserved)
{
	struct exynos_iommu_domain *priv = domain->priv;
	struct exynos_iommu_owner *owner;
	unsigned long flags;

	itype %= 16;

	if (itype == SYSMMU_PAGEFAULT) {
		struct exynos_iovmm *vmm_data;
		sysmmu_pte_t *sent;
		sysmmu_pte_t *pent;

		BUG_ON(priv->pgtable == NULL);

		spin_lock_irqsave(&priv->pgtablelock, flags);

		sent = section_entry(priv->pgtable, fault_addr);
		if (!lv1ent_page(sent)) {
			pent = kmem_cache_zalloc(lv2table_kmem_cache,
						 GFP_ATOMIC);
			if (!pent)
				return -ENOMEM;

			*sent = mk_lv1ent_page(__pa(pent));
			pgtable_flush(sent, sent + 1);
		}
		pent = page_entry(sent, fault_addr);
		if (lv2ent_fault(pent)) {
			*pent = mk_lv2ent_spage(fault_page);
			pgtable_flush(pent, pent + 1);
		} else {
			pr_err("[%s] 0x%lx by '%s' is already mapped\n",
				sysmmu_fault_name[itype], fault_addr,
				dev_name(dev));
		}

		spin_unlock_irqrestore(&priv->pgtablelock, flags);

		owner = dev->archdata.iommu;
		vmm_data = (struct exynos_iovmm *)owner->vmm_data;
		if (find_iovm_region(vmm_data, fault_addr)) {
			pr_err("[%s] 0x%lx by '%s' is remapped\n",
				sysmmu_fault_name[itype],
				fault_addr, dev_name(dev));
		} else {
			pr_err("[%s] '%s' accessed unmapped address(0x%lx)\n",
				sysmmu_fault_name[itype], dev_name(dev),
				fault_addr);
		}
	} else if (itype == SYSMMU_L1TLB_MULTIHIT) {
		spin_lock_irqsave(&priv->lock, flags);
		list_for_each_entry(owner, &priv->clients, client)
			sysmmu_tlb_invalidate_entry(owner->dev,
						(dma_addr_t)fault_addr, true);
		spin_unlock_irqrestore(&priv->lock, flags);

		pr_err("[%s] occured at 0x%lx by '%s'\n",
			sysmmu_fault_name[itype], fault_addr, dev_name(dev));
	} else {
		return -ENOSYS;
	}

	return 0;
}
#else
int recover_fault_handler (struct iommu_domain *domain,
				struct device *dev, unsigned long fault_addr,
				int itype, void *reserved)
{
	return -ENOSYS;
}
#endif

/* called by exynos5-iommu.c and exynos7-iommu.c */
#define PB_CFG_MASK	0x11111;
int __prepare_prefetch_buffers_by_plane(struct sysmmu_drvdata *drvdata,
				struct sysmmu_prefbuf prefbuf[], int num_pb,
				int inplanes, int onplanes,
				int ipoption, int opoption)
{
	int ret_num_pb = 0;
	int i = 0;
	struct exynos_iovmm *vmm;

	if (!drvdata->master || !drvdata->master->archdata.iommu) {
		dev_err(drvdata->sysmmu, "%s: No master device is specified\n",
					__func__);
		return 0;
	}

	vmm = ((struct exynos_iommu_owner *)
			(drvdata->master->archdata.iommu))->vmm_data;
	if (!vmm)
		return 0; /* No VMM information to set prefetch buffers */

	if (!inplanes && !onplanes) {
		inplanes = vmm->inplanes;
		onplanes = vmm->onplanes;
	}

	ipoption &= PB_CFG_MASK;
	opoption &= PB_CFG_MASK;

	if (drvdata->prop & SYSMMU_PROP_READ) {
		ret_num_pb = min(inplanes, num_pb);
		for (i = 0; i < ret_num_pb; i++) {
			prefbuf[i].base = vmm->iova_start[i];
			prefbuf[i].size = vmm->iovm_size[i];
			prefbuf[i].config = ipoption;
		}
	}

	if ((drvdata->prop & SYSMMU_PROP_WRITE) &&
				(ret_num_pb < num_pb) && (onplanes > 0)) {
		for (i = 0; i < min(num_pb - ret_num_pb, onplanes); i++) {
			prefbuf[ret_num_pb + i].base =
					vmm->iova_start[vmm->inplanes + i];
			prefbuf[ret_num_pb + i].size =
					vmm->iovm_size[vmm->inplanes + i];
			prefbuf[ret_num_pb + i].config = opoption;
		}

		ret_num_pb += i;
	}

	if (drvdata->prop & SYSMMU_PROP_WINDOW_MASK) {
		unsigned long prop = (drvdata->prop & SYSMMU_PROP_WINDOW_MASK)
						>> SYSMMU_PROP_WINDOW_SHIFT;
		BUG_ON(ret_num_pb != 0);
		for (i = 0; (i < (vmm->inplanes + vmm->onplanes)) &&
						(ret_num_pb < num_pb); i++) {
			if (prop & 1) {
				prefbuf[ret_num_pb].base = vmm->iova_start[i];
				prefbuf[ret_num_pb].size = vmm->iovm_size[i];
				prefbuf[ret_num_pb].config = ipoption;
				ret_num_pb++;
			}
			prop >>= 1;
			if (prop == 0)
				break;
		}
	}

	return ret_num_pb;
}

void sysmmu_set_prefetch_buffer_by_region(struct device *dev,
			struct sysmmu_prefbuf pb_reg[], unsigned int num_reg)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;
	unsigned long flags;

	if (!dev->archdata.iommu) {
		dev_err(dev, "%s: No System MMU is configured\n", __func__);
		return;
	}

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock(&drvdata->lock);

		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			spin_unlock(&drvdata->lock);
			continue;
		}

		__master_clk_enable(drvdata);

		if (sysmmu_block(drvdata->sfrbase)) {
			__exynos_sysmmu_set_prefbuf_by_region(drvdata, pb_reg, num_reg);
			sysmmu_unblock(drvdata->sfrbase);
		}

		__master_clk_disable(drvdata);

		spin_unlock(&drvdata->lock);
	}

	spin_unlock_irqrestore(&owner->lock, flags);
}

int sysmmu_set_prefetch_buffer_by_plane(struct device *dev,
			unsigned int inplanes, unsigned int onplanes,
			unsigned int ipoption, unsigned int opoption)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct exynos_iovmm *vmm;
	struct sysmmu_list_data *list;
	unsigned long flags;

	if (!dev->archdata.iommu) {
		dev_err(dev, "%s: No System MMU is configured\n", __func__);
		return -EINVAL;
	}

	vmm = exynos_get_iovmm(dev);
	if (!vmm) {
		dev_err(dev, "%s: IOVMM is not configured\n", __func__);
		return -EINVAL;
	}

	if ((inplanes > vmm->inplanes) || (onplanes > vmm->onplanes)) {
		dev_err(dev, "%s: Given planes [%d, %d] exceeds [%d, %d]\n",
				__func__, inplanes, onplanes,
				vmm->inplanes, vmm->onplanes);
		return -EINVAL;
	}

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock(&drvdata->lock);

		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			spin_unlock(&drvdata->lock);
			continue;
		}

		__master_clk_enable(drvdata);

		if (sysmmu_block(drvdata->sfrbase)) {
			__exynos_sysmmu_set_prefbuf_by_plane(drvdata,
					inplanes, onplanes, ipoption, opoption);
			sysmmu_unblock(drvdata->sfrbase);
		}

		__master_clk_disable(drvdata);

		spin_unlock(&drvdata->lock);
	}

	spin_unlock_irqrestore(&owner->lock, flags);

	return 0;
}

static void __sysmmu_set_ptwqos(struct sysmmu_drvdata *data)
{
	u32 cfg;

	if (!sysmmu_block(data->sfrbase))
		return;

	cfg = __raw_readl(data->sfrbase + REG_MMU_CFG);
	cfg &= ~CFG_QOS(15); /* clearing PTW_QOS field */

	/*
	 * PTW_QOS of System MMU 1.x ~ 3.x are all overridable
	 * in __sysmmu_init_config()
	 */
	if (__raw_sysmmu_version(data->sfrbase) < MAKE_MMU_VER(5, 0))
		cfg |= CFG_QOS(data->qos);
	else if (!(data->qos < 0))
		cfg |= CFG_QOS_OVRRIDE | CFG_QOS(data->qos);
	else
		cfg &= ~CFG_QOS_OVRRIDE;

	__raw_writel(cfg, data->sfrbase + REG_MMU_CFG);
	sysmmu_unblock(data->sfrbase);
}

static void __sysmmu_set_qos(struct device *dev, unsigned int qosval)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;
	unsigned long flags;

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *data;
		data = dev_get_drvdata(list->sysmmu);
		spin_lock(&data->lock);
		data->qos = qosval;
		if (is_sysmmu_really_enabled(data)) {
			__master_clk_enable(data);
			__sysmmu_set_ptwqos(data);
			__master_clk_disable(data);
		}
		spin_unlock(&data->lock);
	}

	spin_unlock_irqrestore(&owner->lock, flags);
}

void sysmmu_set_qos(struct device *dev, unsigned int qos)
{
	__sysmmu_set_qos(dev, (qos > 15) ? 15 : qos);
}

void sysmmu_reset_qos(struct device *dev)
{
	__sysmmu_set_qos(dev, DEFAULT_QOS_VALUE);
}

void exynos_sysmmu_set_df(struct device *dev, dma_addr_t iova)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;
	unsigned long flags;
	struct exynos_iovmm *vmm;
	int plane;

	BUG_ON(!has_sysmmu(dev));

	vmm = exynos_get_iovmm(dev);
	if (!vmm) {
		dev_err(dev, "%s: IOVMM not found\n", __func__);
		return;
	}

	plane = find_iovmm_plane(vmm, iova);
	if (plane < 0) {
		dev_err(dev, "%s: IOVA %#x is out of IOVMM\n", __func__, iova);
		return;
	}

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock(&drvdata->lock);

		if (is_sysmmu_active(drvdata) && drvdata->runtime_active) {
			__master_clk_enable(drvdata);
			if (drvdata->prop & SYSMMU_PROP_WINDOW_MASK) {
				unsigned long prop;
				prop = drvdata->prop & SYSMMU_PROP_WINDOW_MASK;
				prop >>= SYSMMU_PROP_WINDOW_SHIFT;
				if (prop & (1 << plane))
					__exynos_sysmmu_set_df(drvdata, iova);
			} else {
				__exynos_sysmmu_set_df(drvdata, iova);
			}
			__master_clk_disable(drvdata);
		}
		spin_unlock(&drvdata->lock);
	}

	spin_unlock_irqrestore(&owner->lock, flags);
}

void exynos_sysmmu_release_df(struct device *dev)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct sysmmu_list_data *list;
	unsigned long flags;

	BUG_ON(!has_sysmmu(dev));

	spin_lock_irqsave(&owner->lock, flags);

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock(&drvdata->lock);
		if (is_sysmmu_active(drvdata) && drvdata->runtime_active) {
			__master_clk_enable(drvdata);
			__exynos_sysmmu_release_df(drvdata);
			__master_clk_disable(drvdata);
		}
		spin_unlock(&drvdata->lock);
	}

	spin_unlock_irqrestore(&owner->lock, flags);
}

static int __init __sysmmu_init_clock(struct device *sysmmu,
					struct sysmmu_drvdata *drvdata)
{
	int ret;

	drvdata->clk = devm_clk_get(sysmmu, "sysmmu");
	if (IS_ERR(drvdata->clk)) {
		if (PTR_ERR(drvdata->clk) == -ENOENT) {
			dev_info(sysmmu, "No gating clock found.\n");
			drvdata->clk = NULL;
			return 0;
		}

		dev_err(sysmmu, "Failed get sysmmu clock\n");
		return PTR_ERR(drvdata->clk);
	}

	ret = clk_prepare(drvdata->clk);
	if (ret) {
		dev_err(sysmmu, "Failed to prepare sysmmu clock\n");
		return ret;
	}

	drvdata->clk_master = devm_clk_get(sysmmu, "master");
	if (PTR_ERR(drvdata->clk_master) == -ENOENT) {
		drvdata->clk_master = NULL;
		return 0;
	} else if (IS_ERR(drvdata->clk_master)) {
		dev_err(sysmmu, "Failed to get master clock\n");
		clk_unprepare(drvdata->clk);
		return PTR_ERR(drvdata->clk_master);
	}

	ret = clk_prepare(drvdata->clk_master);
	if (ret) {
		clk_unprepare(drvdata->clk);
		dev_err(sysmmu, "Failed to prepare master clock\n");
		return ret;
	}

	return 0;
}

static int __init __sysmmu_init_master(struct device *dev)
{
	int ret;
	int i = 0;
	struct device_node *node;

	while ((node = of_parse_phandle(dev->of_node, "mmu-masters", i++))) {
		struct platform_device *master = of_find_device_by_node(node);
		struct exynos_iommu_owner *owner;
		struct sysmmu_list_data *list_data;

		if (!master) {
			dev_err(dev, "%s: mmu-master '%s' not found\n",
				__func__, node->name);
			ret = -EINVAL;
			goto err;
		}

		owner = master->dev.archdata.iommu;
		if (!owner) {
			owner = devm_kzalloc(dev, sizeof(*owner), GFP_KERNEL);
			if (!owner) {
				dev_err(dev,
				"%s: Failed to allocate owner structure\n",
				__func__);
				ret = -ENOMEM;
				goto err;
			}

			INIT_LIST_HEAD(&owner->mmu_list);
			INIT_LIST_HEAD(&owner->client);
			INIT_LIST_HEAD(&owner->entry);
			owner->dev = &master->dev;
			spin_lock_init(&owner->lock);

			master->dev.archdata.iommu = owner;
			list_add_tail(&owner->entry, &sysmmu_owner_list);
		}

		list_data = devm_kzalloc(dev, sizeof(*list_data), GFP_KERNEL);
		if (!list_data) {
			dev_err(dev,
				"%s: Failed to allocate sysmmu_list_data\n",
				__func__);
			ret = -ENOMEM;
			goto err;
		}

		INIT_LIST_HEAD(&list_data->node);
		list_data->sysmmu = dev;

		/*
		 * System MMUs are attached in the order of the presence
		 * in device tree
		 */
		list_add_tail(&list_data->node, &owner->mmu_list);
		dev_info(dev, "--> %s\n", dev_name(&master->dev));
	}

	return 0;
err:
	while ((node = of_parse_phandle(dev->of_node, "mmu-masters", i++))) {
		struct platform_device *master = of_find_device_by_node(node);
		struct exynos_iommu_owner *owner;
		struct sysmmu_list_data *list_data;

		if (!master)
			continue;

		owner = master->dev.archdata.iommu;
		if (!owner)
			continue;

		list_for_each_entry(list_data, &owner->mmu_list, node) {
			if (list_data->sysmmu == dev) {
				list_del(&list_data->node);
				kfree(list_data);
				break;
			}
		}
	}

	return ret;
}

static const char * const sysmmu_prop_opts[] = {
	[SYSMMU_PROP_RESERVED]		= "Reserved",
	[SYSMMU_PROP_READ]		= "r",
	[SYSMMU_PROP_WRITE]		= "w",
	[SYSMMU_PROP_READWRITE]		= "rw",	/* default */
};

static int __init __sysmmu_init_prop(struct device *sysmmu,
				     struct sysmmu_drvdata *drvdata)
{
	struct device_node *prop_node;
	const char *s;
	int winmap = 0;
	unsigned int qos = DEFAULT_QOS_VALUE;
	int ret;

	drvdata->prop = SYSMMU_PROP_READWRITE;

	ret = of_property_read_u32_index(sysmmu->of_node, "qos", 0, &qos);

	if ((ret == 0) && (qos > 15)) {
		dev_err(sysmmu, "%s: Invalid QoS value %d specified\n",
				__func__, qos);
		qos = DEFAULT_QOS_VALUE;
	}

	drvdata->qos = (short)qos;

	prop_node = of_get_child_by_name(sysmmu->of_node, "prop-map");
	if (!prop_node)
		return 0;

	if (!of_property_read_string(prop_node, "iomap", &s)) {
		int val;
		for (val = 1; val < ARRAY_SIZE(sysmmu_prop_opts); val++) {
			if (!strcasecmp(s, sysmmu_prop_opts[val])) {
				drvdata->prop &= ~SYSMMU_PROP_RW_MASK;
				drvdata->prop |= val;
				break;
			}
		}
	} else if (!of_property_read_u32_index(
					prop_node, "winmap", 0, &winmap)) {
		if (winmap) {
			drvdata->prop &= ~SYSMMU_PROP_RW_MASK;
			drvdata->prop |= winmap << SYSMMU_PROP_WINDOW_SHIFT;
		}
	}

	if (!of_property_read_string(prop_node, "tlbinv-nonblock", &s))
		if (strnicmp(s, "yes", 3) == 0)
			drvdata->prop |= SYSMMU_PROP_NONBLOCK_TLBINV;

	if (!of_property_read_string(prop_node, "block-stop", &s))
		if (strnicmp(s, "yes", 3) == 0)
			drvdata->prop |= SYSMMU_PROP_STOP_BLOCK;
	return 0;
}

static int __init __sysmmu_setup(struct device *sysmmu,
				struct sysmmu_drvdata *drvdata)
{
	int ret;

	ret = __sysmmu_init_prop(sysmmu, drvdata);
	if (ret) {
		dev_err(sysmmu, "Failed to initialize sysmmu properties\n");
		return ret;
	}

	ret = __sysmmu_init_clock(sysmmu, drvdata);
	if (ret) {
		dev_err(sysmmu, "Failed to initialize gating clocks\n");
		return ret;
	}

	ret = __sysmmu_init_master(sysmmu);
	if (ret) {
		if (drvdata->clk)
			clk_unprepare(drvdata->clk);
		if (drvdata->clk_master)
			clk_unprepare(drvdata->clk_master);
		dev_err(sysmmu, "Failed to initialize master device.\n");
	}

	return ret;
}

static int __init exynos_sysmmu_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct sysmmu_drvdata *data;
	struct resource *res;

	data = devm_kzalloc(dev, sizeof(*data) , GFP_KERNEL);
	if (!data) {
		dev_err(dev, "Not enough memory\n");
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Unable to find IOMEM region\n");
		return -ENOENT;
	}

	data->sfrbase = devm_request_and_ioremap(dev, res);
	if (!data->sfrbase) {
		dev_err(dev, "Unable to map IOMEM @ PA:%#x\n", res->start);
		return -EBUSY;
	}

	ret = platform_get_irq(pdev, 0);
	if (ret <= 0) {
		dev_err(dev, "Unable to find IRQ resource\n");
		return ret;
	}

	ret = devm_request_irq(dev, ret, exynos_sysmmu_irq, 0,
				dev_name(dev), data);
	if (ret) {
		dev_err(dev, "Unabled to register interrupt handler\n");
		return ret;
	}

	pm_runtime_enable(dev);

	ret = exynos_iommu_init_event_log(SYSMMU_DRVDATA_TO_LOG(data),
					SYSMMU_LOG_LEN);
	if (!ret)
		sysmmu_add_log_to_debugfs(exynos_sysmmu_debugfs_root,
				SYSMMU_DRVDATA_TO_LOG(data), dev_name(dev));
	else
		return ret;

	ret = __sysmmu_setup(dev, data);
	if (!ret) {
		data->runtime_active = !pm_runtime_enabled(dev);
		data->sysmmu = dev;
		INIT_LIST_HEAD(&data->entry);
		spin_lock_init(&data->lock);

		list_add_tail(&data->entry, &sysmmu_drvdata_list);
		platform_set_drvdata(pdev, data);

		dev_info(dev, "[OK]\n");
	}

	return ret;
}

#ifdef CONFIG_OF
static struct of_device_id sysmmu_of_match[] __initconst = {
	{ .compatible = SYSMMU_OF_COMPAT_STRING, },
	{ },
};
#endif

static struct platform_driver exynos_sysmmu_driver __refdata = {
	.probe		= exynos_sysmmu_probe,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= MODULE_NAME,
		.of_match_table = of_match_ptr(sysmmu_of_match),
	}
};

static int exynos_iommu_domain_init(struct iommu_domain *domain)
{
	struct exynos_iommu_domain *priv;
	int i;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pgtable = (sysmmu_pte_t *)__get_free_pages(
						GFP_KERNEL | __GFP_ZERO, 2);
	if (!priv->pgtable)
		goto err_pgtable;

	priv->lv2entcnt = (short *)__get_free_pages(
						GFP_KERNEL | __GFP_ZERO, 1);
	if (!priv->lv2entcnt)
		goto err_counter;

	if (exynos_iommu_init_event_log(IOMMU_PRIV_TO_LOG(priv), IOMMU_LOG_LEN))
		goto err_init_event_log;

	for (i = 0; i < NUM_LV1ENTRIES; i += 8) {
		priv->pgtable[i + 0] = ZERO_LV2LINK;
		priv->pgtable[i + 1] = ZERO_LV2LINK;
		priv->pgtable[i + 2] = ZERO_LV2LINK;
		priv->pgtable[i + 3] = ZERO_LV2LINK;
		priv->pgtable[i + 4] = ZERO_LV2LINK;
		priv->pgtable[i + 5] = ZERO_LV2LINK;
		priv->pgtable[i + 6] = ZERO_LV2LINK;
		priv->pgtable[i + 7] = ZERO_LV2LINK;
	}

	pgtable_flush(priv->pgtable, priv->pgtable + NUM_LV1ENTRIES);

	spin_lock_init(&priv->lock);
	spin_lock_init(&priv->pgtablelock);
	INIT_LIST_HEAD(&priv->clients);

	domain->priv = priv;
	domain->handler = recover_fault_handler;

	return 0;

err_init_event_log:
	free_pages((unsigned long)priv->lv2entcnt, 1);
=======
#include <mach/sysmmu.h>

/* We does not consider super section mapping (16MB) */
#define SECT_ORDER 20
#define LPAGE_ORDER 16
#define SPAGE_ORDER 12

#define SECT_SIZE (1 << SECT_ORDER)
#define LPAGE_SIZE (1 << LPAGE_ORDER)
#define SPAGE_SIZE (1 << SPAGE_ORDER)

#define SECT_MASK (~(SECT_SIZE - 1))
#define LPAGE_MASK (~(LPAGE_SIZE - 1))
#define SPAGE_MASK (~(SPAGE_SIZE - 1))

#define lv1ent_fault(sent) (((*(sent) & 3) == 0) || ((*(sent) & 3) == 3))
#define lv1ent_page(sent) ((*(sent) & 3) == 1)
#define lv1ent_section(sent) ((*(sent) & 3) == 2)

#define lv2ent_fault(pent) ((*(pent) & 3) == 0)
#define lv2ent_small(pent) ((*(pent) & 2) == 2)
#define lv2ent_large(pent) ((*(pent) & 3) == 1)

#define section_phys(sent) (*(sent) & SECT_MASK)
#define section_offs(iova) ((iova) & 0xFFFFF)
#define lpage_phys(pent) (*(pent) & LPAGE_MASK)
#define lpage_offs(iova) ((iova) & 0xFFFF)
#define spage_phys(pent) (*(pent) & SPAGE_MASK)
#define spage_offs(iova) ((iova) & 0xFFF)

#define lv1ent_offset(iova) ((iova) >> SECT_ORDER)
#define lv2ent_offset(iova) (((iova) & 0xFF000) >> SPAGE_ORDER)

#define NUM_LV1ENTRIES 4096
#define NUM_LV2ENTRIES 256

#define LV2TABLE_SIZE (NUM_LV2ENTRIES * sizeof(long))

#define SPAGES_PER_LPAGE (LPAGE_SIZE / SPAGE_SIZE)

#define lv2table_base(sent) (*(sent) & 0xFFFFFC00)

#define mk_lv1ent_sect(pa) ((pa) | 2)
#define mk_lv1ent_page(pa) ((pa) | 1)
#define mk_lv2ent_lpage(pa) ((pa) | 1)
#define mk_lv2ent_spage(pa) ((pa) | 2)

#define CTRL_ENABLE	0x5
#define CTRL_BLOCK	0x7
#define CTRL_DISABLE	0x0

#define REG_MMU_CTRL		0x000
#define REG_MMU_CFG		0x004
#define REG_MMU_STATUS		0x008
#define REG_MMU_FLUSH		0x00C
#define REG_MMU_FLUSH_ENTRY	0x010
#define REG_PT_BASE_ADDR	0x014
#define REG_INT_STATUS		0x018
#define REG_INT_CLEAR		0x01C

#define REG_PAGE_FAULT_ADDR	0x024
#define REG_AW_FAULT_ADDR	0x028
#define REG_AR_FAULT_ADDR	0x02C
#define REG_DEFAULT_SLAVE_ADDR	0x030

#define REG_MMU_VERSION		0x034

#define REG_PB0_SADDR		0x04C
#define REG_PB0_EADDR		0x050
#define REG_PB1_SADDR		0x054
#define REG_PB1_EADDR		0x058

static unsigned long *section_entry(unsigned long *pgtable, unsigned long iova)
{
	return pgtable + lv1ent_offset(iova);
}

static unsigned long *page_entry(unsigned long *sent, unsigned long iova)
{
	return (unsigned long *)__va(lv2table_base(sent)) + lv2ent_offset(iova);
}

enum exynos_sysmmu_inttype {
	SYSMMU_PAGEFAULT,
	SYSMMU_AR_MULTIHIT,
	SYSMMU_AW_MULTIHIT,
	SYSMMU_BUSERROR,
	SYSMMU_AR_SECURITY,
	SYSMMU_AR_ACCESS,
	SYSMMU_AW_SECURITY,
	SYSMMU_AW_PROTECTION, /* 7 */
	SYSMMU_FAULT_UNKNOWN,
	SYSMMU_FAULTS_NUM
};

/*
 * @itype: type of fault.
 * @pgtable_base: the physical address of page table base. This is 0 if @itype
 *                is SYSMMU_BUSERROR.
 * @fault_addr: the device (virtual) address that the System MMU tried to
 *             translated. This is 0 if @itype is SYSMMU_BUSERROR.
 */
typedef int (*sysmmu_fault_handler_t)(enum exynos_sysmmu_inttype itype,
			unsigned long pgtable_base, unsigned long fault_addr);

static unsigned short fault_reg_offset[SYSMMU_FAULTS_NUM] = {
	REG_PAGE_FAULT_ADDR,
	REG_AR_FAULT_ADDR,
	REG_AW_FAULT_ADDR,
	REG_DEFAULT_SLAVE_ADDR,
	REG_AR_FAULT_ADDR,
	REG_AR_FAULT_ADDR,
	REG_AW_FAULT_ADDR,
	REG_AW_FAULT_ADDR
};

static char *sysmmu_fault_name[SYSMMU_FAULTS_NUM] = {
	"PAGE FAULT",
	"AR MULTI-HIT FAULT",
	"AW MULTI-HIT FAULT",
	"BUS ERROR",
	"AR SECURITY PROTECTION FAULT",
	"AR ACCESS PROTECTION FAULT",
	"AW SECURITY PROTECTION FAULT",
	"AW ACCESS PROTECTION FAULT",
	"UNKNOWN FAULT"
};

struct exynos_iommu_domain {
	struct list_head clients; /* list of sysmmu_drvdata.node */
	unsigned long *pgtable; /* lv1 page table, 16KB */
	short *lv2entcnt; /* free lv2 entry counter for each section */
	spinlock_t lock; /* lock for this structure */
	spinlock_t pgtablelock; /* lock for modifying page table @ pgtable */
};

struct sysmmu_drvdata {
	struct list_head node; /* entry of exynos_iommu_domain.clients */
	struct device *sysmmu;	/* System MMU's device descriptor */
	struct device *dev;	/* Owner of system MMU */
	char *dbgname;
	int nsfrs;
	void __iomem **sfrbases;
	struct clk *clk[2];
	int activations;
	rwlock_t lock;
	struct iommu_domain *domain;
	sysmmu_fault_handler_t fault_handler;
	unsigned long pgtable;
};

static bool set_sysmmu_active(struct sysmmu_drvdata *data)
{
	/* return true if the System MMU was not active previously
	   and it needs to be initialized */
	return ++data->activations == 1;
}

static bool set_sysmmu_inactive(struct sysmmu_drvdata *data)
{
	/* return true if the System MMU is needed to be disabled */
	BUG_ON(data->activations < 1);
	return --data->activations == 0;
}

static bool is_sysmmu_active(struct sysmmu_drvdata *data)
{
	return data->activations > 0;
}

static void sysmmu_unblock(void __iomem *sfrbase)
{
	__raw_writel(CTRL_ENABLE, sfrbase + REG_MMU_CTRL);
}

static bool sysmmu_block(void __iomem *sfrbase)
{
	int i = 120;

	__raw_writel(CTRL_BLOCK, sfrbase + REG_MMU_CTRL);
	while ((i > 0) && !(__raw_readl(sfrbase + REG_MMU_STATUS) & 1))
		--i;

	if (!(__raw_readl(sfrbase + REG_MMU_STATUS) & 1)) {
		sysmmu_unblock(sfrbase);
		return false;
	}

	return true;
}

static void __sysmmu_tlb_invalidate(void __iomem *sfrbase)
{
	__raw_writel(0x1, sfrbase + REG_MMU_FLUSH);
}

static void __sysmmu_tlb_invalidate_entry(void __iomem *sfrbase,
						unsigned long iova)
{
	__raw_writel((iova & SPAGE_MASK) | 1, sfrbase + REG_MMU_FLUSH_ENTRY);
}

static void __sysmmu_set_ptbase(void __iomem *sfrbase,
				       unsigned long pgd)
{
	__raw_writel(0x1, sfrbase + REG_MMU_CFG); /* 16KB LV1, LRU */
	__raw_writel(pgd, sfrbase + REG_PT_BASE_ADDR);

	__sysmmu_tlb_invalidate(sfrbase);
}

static void __sysmmu_set_prefbuf(void __iomem *sfrbase, unsigned long base,
						unsigned long size, int idx)
{
	__raw_writel(base, sfrbase + REG_PB0_SADDR + idx * 8);
	__raw_writel(size - 1 + base,  sfrbase + REG_PB0_EADDR + idx * 8);
}

void exynos_sysmmu_set_prefbuf(struct device *dev,
				unsigned long base0, unsigned long size0,
				unsigned long base1, unsigned long size1)
{
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);
	unsigned long flags;
	int i;

	BUG_ON((base0 + size0) <= base0);
	BUG_ON((size1 > 0) && ((base1 + size1) <= base1));

	read_lock_irqsave(&data->lock, flags);
	if (!is_sysmmu_active(data))
		goto finish;

	for (i = 0; i < data->nsfrs; i++) {
		if ((readl(data->sfrbases[i] + REG_MMU_VERSION) >> 28) == 3) {
			if (!sysmmu_block(data->sfrbases[i]))
				continue;

			if (size1 == 0) {
				if (size0 <= SZ_128K) {
					base1 = base0;
					size1 = size0;
				} else {
					size1 = size0 -
						ALIGN(size0 / 2, SZ_64K);
					size0 = size0 - size1;
					base1 = base0 + size0;
				}
			}

			__sysmmu_set_prefbuf(
					data->sfrbases[i], base0, size0, 0);
			__sysmmu_set_prefbuf(
					data->sfrbases[i], base1, size1, 1);

			sysmmu_unblock(data->sfrbases[i]);
		}
	}
finish:
	read_unlock_irqrestore(&data->lock, flags);
}

static void __set_fault_handler(struct sysmmu_drvdata *data,
					sysmmu_fault_handler_t handler)
{
	unsigned long flags;

	write_lock_irqsave(&data->lock, flags);
	data->fault_handler = handler;
	write_unlock_irqrestore(&data->lock, flags);
}

void exynos_sysmmu_set_fault_handler(struct device *dev,
					sysmmu_fault_handler_t handler)
{
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);

	__set_fault_handler(data, handler);
}

static int default_fault_handler(enum exynos_sysmmu_inttype itype,
		     unsigned long pgtable_base, unsigned long fault_addr)
{
	unsigned long *ent;

	if ((itype >= SYSMMU_FAULTS_NUM) || (itype < SYSMMU_PAGEFAULT))
		itype = SYSMMU_FAULT_UNKNOWN;

	pr_err("%s occurred at 0x%lx(Page table base: 0x%lx)\n",
			sysmmu_fault_name[itype], fault_addr, pgtable_base);

	ent = section_entry(__va(pgtable_base), fault_addr);
	pr_err("\tLv1 entry: 0x%lx\n", *ent);

	if (lv1ent_page(ent)) {
		ent = page_entry(ent, fault_addr);
		pr_err("\t Lv2 entry: 0x%lx\n", *ent);
	}

	pr_err("Generating Kernel OOPS... because it is unrecoverable.\n");

	BUG();

	return 0;
}

static irqreturn_t exynos_sysmmu_irq(int irq, void *dev_id)
{
	/* SYSMMU is in blocked when interrupt occurred. */
	struct sysmmu_drvdata *data = dev_id;
	struct resource *irqres;
	struct platform_device *pdev;
	enum exynos_sysmmu_inttype itype;
	unsigned long addr = -1;

	int i, ret = -ENOSYS;

	read_lock(&data->lock);

	WARN_ON(!is_sysmmu_active(data));

	pdev = to_platform_device(data->sysmmu);
	for (i = 0; i < (pdev->num_resources / 2); i++) {
		irqres = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if (irqres && ((int)irqres->start == irq))
			break;
	}

	if (i == pdev->num_resources) {
		itype = SYSMMU_FAULT_UNKNOWN;
	} else {
		itype = (enum exynos_sysmmu_inttype)
			__ffs(__raw_readl(data->sfrbases[i] + REG_INT_STATUS));
		if (WARN_ON(!((itype >= 0) && (itype < SYSMMU_FAULT_UNKNOWN))))
			itype = SYSMMU_FAULT_UNKNOWN;
		else
			addr = __raw_readl(
				data->sfrbases[i] + fault_reg_offset[itype]);
	}

	if (data->domain)
		ret = report_iommu_fault(data->domain, data->dev,
				addr, itype);

	if ((ret == -ENOSYS) && data->fault_handler) {
		unsigned long base = data->pgtable;
		if (itype != SYSMMU_FAULT_UNKNOWN)
			base = __raw_readl(
					data->sfrbases[i] + REG_PT_BASE_ADDR);
		ret = data->fault_handler(itype, base, addr);
	}

	if (!ret && (itype != SYSMMU_FAULT_UNKNOWN))
		__raw_writel(1 << itype, data->sfrbases[i] + REG_INT_CLEAR);
	else
		dev_dbg(data->sysmmu, "(%s) %s is not handled.\n",
				data->dbgname, sysmmu_fault_name[itype]);

	if (itype != SYSMMU_FAULT_UNKNOWN)
		sysmmu_unblock(data->sfrbases[i]);

	read_unlock(&data->lock);

	return IRQ_HANDLED;
}

static bool __exynos_sysmmu_disable(struct sysmmu_drvdata *data)
{
	unsigned long flags;
	bool disabled = false;
	int i;

	write_lock_irqsave(&data->lock, flags);

	if (!set_sysmmu_inactive(data))
		goto finish;

	for (i = 0; i < data->nsfrs; i++)
		__raw_writel(CTRL_DISABLE, data->sfrbases[i] + REG_MMU_CTRL);

	if (data->clk[1])
		clk_disable(data->clk[1]);
	if (data->clk[0])
		clk_disable(data->clk[0]);

	disabled = true;
	data->pgtable = 0;
	data->domain = NULL;
finish:
	write_unlock_irqrestore(&data->lock, flags);

	if (disabled)
		dev_dbg(data->sysmmu, "(%s) Disabled\n", data->dbgname);
	else
		dev_dbg(data->sysmmu, "(%s) %d times left to be disabled\n",
					data->dbgname, data->activations);

	return disabled;
}

/* __exynos_sysmmu_enable: Enables System MMU
 *
 * returns -error if an error occurred and System MMU is not enabled,
 * 0 if the System MMU has been just enabled and 1 if System MMU was already
 * enabled before.
 */
static int __exynos_sysmmu_enable(struct sysmmu_drvdata *data,
			unsigned long pgtable, struct iommu_domain *domain)
{
	int i, ret = 0;
	unsigned long flags;

	write_lock_irqsave(&data->lock, flags);

	if (!set_sysmmu_active(data)) {
		if (WARN_ON(pgtable != data->pgtable)) {
			ret = -EBUSY;
			set_sysmmu_inactive(data);
		} else {
			ret = 1;
		}

		dev_dbg(data->sysmmu, "(%s) Already enabled\n", data->dbgname);
		goto finish;
	}

	if (data->clk[0])
		clk_enable(data->clk[0]);
	if (data->clk[1])
		clk_enable(data->clk[1]);

	data->pgtable = pgtable;

	for (i = 0; i < data->nsfrs; i++) {
		__sysmmu_set_ptbase(data->sfrbases[i], pgtable);

		if ((readl(data->sfrbases[i] + REG_MMU_VERSION) >> 28) == 3) {
			/* System MMU version is 3.x */
			__raw_writel((1 << 12) | (2 << 28),
					data->sfrbases[i] + REG_MMU_CFG);
			__sysmmu_set_prefbuf(data->sfrbases[i], 0, -1, 0);
			__sysmmu_set_prefbuf(data->sfrbases[i], 0, -1, 1);
		}

		__raw_writel(CTRL_ENABLE, data->sfrbases[i] + REG_MMU_CTRL);
	}

	data->domain = domain;

	dev_dbg(data->sysmmu, "(%s) Enabled\n", data->dbgname);
finish:
	write_unlock_irqrestore(&data->lock, flags);

	return ret;
}

int exynos_sysmmu_enable(struct device *dev, unsigned long pgtable)
{
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);
	int ret;

	BUG_ON(!memblock_is_memory(pgtable));

	ret = pm_runtime_get_sync(data->sysmmu);
	if (ret < 0) {
		dev_dbg(data->sysmmu, "(%s) Failed to enable\n", data->dbgname);
		return ret;
	}

	ret = __exynos_sysmmu_enable(data, pgtable, NULL);
	if (WARN_ON(ret < 0)) {
		pm_runtime_put(data->sysmmu);
		dev_err(data->sysmmu,
			"(%s) Already enabled with page table %#lx\n",
			data->dbgname, data->pgtable);
	} else {
		data->dev = dev;
	}

	return ret;
}

static bool exynos_sysmmu_disable(struct device *dev)
{
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);
	bool disabled;

	disabled = __exynos_sysmmu_disable(data);
	pm_runtime_put(data->sysmmu);

	return disabled;
}

static void sysmmu_tlb_invalidate_entry(struct device *dev, unsigned long iova)
{
	unsigned long flags;
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);

	read_lock_irqsave(&data->lock, flags);

	if (is_sysmmu_active(data)) {
		int i;
		for (i = 0; i < data->nsfrs; i++) {
			if (sysmmu_block(data->sfrbases[i])) {
				__sysmmu_tlb_invalidate_entry(
						data->sfrbases[i], iova);
				sysmmu_unblock(data->sfrbases[i]);
			}
		}
	} else {
		dev_dbg(data->sysmmu,
			"(%s) Disabled. Skipping invalidating TLB.\n",
			data->dbgname);
	}

	read_unlock_irqrestore(&data->lock, flags);
}

void exynos_sysmmu_tlb_invalidate(struct device *dev)
{
	unsigned long flags;
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);

	read_lock_irqsave(&data->lock, flags);

	if (is_sysmmu_active(data)) {
		int i;
		for (i = 0; i < data->nsfrs; i++) {
			if (sysmmu_block(data->sfrbases[i])) {
				__sysmmu_tlb_invalidate(data->sfrbases[i]);
				sysmmu_unblock(data->sfrbases[i]);
			}
		}
	} else {
		dev_dbg(data->sysmmu,
			"(%s) Disabled. Skipping invalidating TLB.\n",
			data->dbgname);
	}

	read_unlock_irqrestore(&data->lock, flags);
}

static int exynos_sysmmu_probe(struct platform_device *pdev)
{
	int i, ret;
	struct device *dev;
	struct sysmmu_drvdata *data;

	dev = &pdev->dev;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_dbg(dev, "Not enough memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	ret = dev_set_drvdata(dev, data);
	if (ret) {
		dev_dbg(dev, "Unabled to initialize driver data\n");
		goto err_init;
	}

	data->nsfrs = pdev->num_resources / 2;
	data->sfrbases = kmalloc(sizeof(*data->sfrbases) * data->nsfrs,
								GFP_KERNEL);
	if (data->sfrbases == NULL) {
		dev_dbg(dev, "Not enough memory\n");
		ret = -ENOMEM;
		goto err_init;
	}

	for (i = 0; i < data->nsfrs; i++) {
		struct resource *res;
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res) {
			dev_dbg(dev, "Unable to find IOMEM region\n");
			ret = -ENOENT;
			goto err_res;
		}

		data->sfrbases[i] = ioremap(res->start, resource_size(res));
		if (!data->sfrbases[i]) {
			dev_dbg(dev, "Unable to map IOMEM @ PA:%#x\n",
							res->start);
			ret = -ENOENT;
			goto err_res;
		}
	}

	for (i = 0; i < data->nsfrs; i++) {
		ret = platform_get_irq(pdev, i);
		if (ret <= 0) {
			dev_dbg(dev, "Unable to find IRQ resource\n");
			goto err_irq;
		}

		ret = request_irq(ret, exynos_sysmmu_irq, 0,
					dev_name(dev), data);
		if (ret) {
			dev_dbg(dev, "Unabled to register interrupt handler\n");
			goto err_irq;
		}
	}

	if (dev_get_platdata(dev)) {
		char *deli, *beg;
		struct sysmmu_platform_data *platdata = dev_get_platdata(dev);

		beg = platdata->clockname;

		for (deli = beg; (*deli != '\0') && (*deli != ','); deli++)
			/* NOTHING */;

		if (*deli == '\0')
			deli = NULL;
		else
			*deli = '\0';

		data->clk[0] = clk_get(dev, beg);
		if (IS_ERR(data->clk[0])) {
			data->clk[0] = NULL;
			dev_dbg(dev, "No clock descriptor registered\n");
		}

		if (data->clk[0] && deli) {
			*deli = ',';
			data->clk[1] = clk_get(dev, deli + 1);
			if (IS_ERR(data->clk[1]))
				data->clk[1] = NULL;
		}

		data->dbgname = platdata->dbgname;
	}

	data->sysmmu = dev;
	rwlock_init(&data->lock);
	INIT_LIST_HEAD(&data->node);

	__set_fault_handler(data, &default_fault_handler);

	if (dev->parent)
		pm_runtime_enable(dev);

	dev_dbg(dev, "(%s) Initialized\n", data->dbgname);
	return 0;
err_irq:
	while (i-- > 0) {
		int irq;

		irq = platform_get_irq(pdev, i);
		free_irq(irq, data);
	}
err_res:
	while (data->nsfrs-- > 0)
		iounmap(data->sfrbases[data->nsfrs]);
	kfree(data->sfrbases);
err_init:
	kfree(data);
err_alloc:
	dev_err(dev, "Failed to initialize\n");
	return ret;
}

static struct platform_driver exynos_sysmmu_driver = {
	.probe		= exynos_sysmmu_probe,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "exynos-sysmmu",
	}
};

static inline void pgtable_flush(void *vastart, void *vaend)
{
	dmac_flush_range(vastart, vaend);
	outer_flush_range(virt_to_phys(vastart),
				virt_to_phys(vaend));
}

static int exynos_iommu_domain_init(struct iommu_domain *domain)
{
	struct exynos_iommu_domain *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pgtable = (unsigned long *)__get_free_pages(
						GFP_KERNEL | __GFP_ZERO, 2);
	if (!priv->pgtable)
		goto err_pgtable;

	priv->lv2entcnt = (short *)__get_free_pages(
						GFP_KERNEL | __GFP_ZERO, 1);
	if (!priv->lv2entcnt)
		goto err_counter;

	pgtable_flush(priv->pgtable, priv->pgtable + NUM_LV1ENTRIES);

	spin_lock_init(&priv->lock);
	spin_lock_init(&priv->pgtablelock);
	INIT_LIST_HEAD(&priv->clients);

	domain->geometry.aperture_start = 0;
	domain->geometry.aperture_end   = ~0UL;
	domain->geometry.force_aperture = true;

	domain->priv = priv;
	return 0;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
err_counter:
	free_pages((unsigned long)priv->pgtable, 2);
err_pgtable:
	kfree(priv);
	return -ENOMEM;
}

static void exynos_iommu_domain_destroy(struct iommu_domain *domain)
{
	struct exynos_iommu_domain *priv = domain->priv;
<<<<<<< HEAD
	struct exynos_iommu_owner *owner;
=======
	struct sysmmu_drvdata *data;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	unsigned long flags;
	int i;

	WARN_ON(!list_empty(&priv->clients));

	spin_lock_irqsave(&priv->lock, flags);

<<<<<<< HEAD
	list_for_each_entry(owner, &priv->clients, client)
		while (!exynos_sysmmu_disable(owner->dev))
			; /* until System MMU is actually disabled */

	while (!list_empty(&priv->clients))
		list_del_init(priv->clients.next);
=======
	list_for_each_entry(data, &priv->clients, node) {
		while (!exynos_sysmmu_disable(data->dev))
			; /* until System MMU is actually disabled */
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_unlock_irqrestore(&priv->lock, flags);

	for (i = 0; i < NUM_LV1ENTRIES; i++)
		if (lv1ent_page(priv->pgtable + i))
<<<<<<< HEAD
			kmem_cache_free(lv2table_kmem_cache,
					__va(lv2table_base(priv->pgtable + i)));

	exynos_iommu_free_event_log(IOMMU_PRIV_TO_LOG(priv), IOMMU_LOG_LEN);
=======
			kfree(__va(lv2table_base(priv->pgtable + i)));
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	free_pages((unsigned long)priv->pgtable, 2);
	free_pages((unsigned long)priv->lv2entcnt, 1);
	kfree(domain->priv);
	domain->priv = NULL;
}

static int exynos_iommu_attach_device(struct iommu_domain *domain,
				   struct device *dev)
{
<<<<<<< HEAD
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
=======
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	struct exynos_iommu_domain *priv = domain->priv;
	unsigned long flags;
	int ret;

<<<<<<< HEAD
	spin_lock_irqsave(&priv->lock, flags);

	ret = __exynos_sysmmu_enable(dev, __pa(priv->pgtable), domain);

	if (ret == 0)
		list_add_tail(&owner->client, &priv->clients);
=======
	ret = pm_runtime_get_sync(data->sysmmu);
	if (ret < 0)
		return ret;

	ret = 0;

	spin_lock_irqsave(&priv->lock, flags);

	ret = __exynos_sysmmu_enable(data, __pa(priv->pgtable), domain);

	if (ret == 0) {
		/* 'data->node' must not be appeared in priv->clients */
		BUG_ON(!list_empty(&data->node));
		data->dev = dev;
		list_add_tail(&data->node, &priv->clients);
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_unlock_irqrestore(&priv->lock, flags);

	if (ret < 0) {
		dev_err(dev, "%s: Failed to attach IOMMU with pgtable %#lx\n",
				__func__, __pa(priv->pgtable));
<<<<<<< HEAD
	} else {
		SYSMMU_EVENT_LOG_IOMMU_ATTACH(IOMMU_PRIV_TO_LOG(priv), dev);
		TRACE_LOG_DEV(dev,
			"%s: Attached new IOMMU with pgtable %#lx %s\n",
			__func__, __pa(priv->pgtable),
			(ret == 0) ? "" : ", again");
=======
		pm_runtime_put(data->sysmmu);
	} else if (ret > 0) {
		dev_dbg(dev, "%s: IOMMU with pgtable 0x%lx already attached\n",
					__func__, __pa(priv->pgtable));
	} else {
		dev_dbg(dev, "%s: Attached new IOMMU with pgtable 0x%lx\n",
					__func__, __pa(priv->pgtable));
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	return ret;
}

static void exynos_iommu_detach_device(struct iommu_domain *domain,
				    struct device *dev)
{
<<<<<<< HEAD
	struct exynos_iommu_owner *owner;
	struct exynos_iommu_domain *priv = domain->priv;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);

	list_for_each_entry(owner, &priv->clients, client) {
		if (owner == dev->archdata.iommu) {
			if (exynos_sysmmu_disable(dev))
				list_del_init(&owner->client);
=======
	struct sysmmu_drvdata *data = dev_get_drvdata(dev->archdata.iommu);
	struct exynos_iommu_domain *priv = domain->priv;
	struct list_head *pos;
	unsigned long flags;
	bool found = false;

	spin_lock_irqsave(&priv->lock, flags);

	list_for_each(pos, &priv->clients) {
		if (list_entry(pos, struct sysmmu_drvdata, node) == data) {
			found = true;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			break;
		}
	}

<<<<<<< HEAD
	spin_unlock_irqrestore(&priv->lock, flags);

	if (owner == dev->archdata.iommu) {
		SYSMMU_EVENT_LOG_IOMMU_DETACH(IOMMU_PRIV_TO_LOG(priv), dev);
		TRACE_LOG_DEV(dev, "%s: Detached IOMMU with pgtable %#lx\n",
					__func__, __pa(priv->pgtable));
	} else {
		dev_err(dev, "%s: No IOMMU is attached\n", __func__);
	}
}

static sysmmu_pte_t *alloc_lv2entry(struct exynos_iommu_domain *priv,
		sysmmu_pte_t *sent, unsigned long iova, short *pgcounter)
{
	if (lv1ent_fault(sent)) {
		sysmmu_pte_t *pent;
		struct exynos_iommu_owner *owner;
		unsigned long flags;

		pent = kmem_cache_zalloc(lv2table_kmem_cache, GFP_ATOMIC);
		BUG_ON((unsigned long)pent & (LV2TABLE_SIZE - 1));
		if (!pent)
			return ERR_PTR(-ENOMEM);

		*sent = mk_lv1ent_page(__pa(pent));
		kmemleak_ignore(pent);
		*pgcounter = NUM_LV2ENTRIES;
		pgtable_flush(pent, pent + NUM_LV2ENTRIES);
		pgtable_flush(sent, sent + 1);
		SYSMMU_EVENT_LOG_IOMMU_ALLOCSLPD(IOMMU_PRIV_TO_LOG(priv),
						iova & SECT_MASK);

		/*
		 * If pretched SLPD is a fault SLPD in zero_l2_table, FLPD cache
		 * caches the address of zero_l2_table. This function replaces
		 * the zero_l2_table with new L2 page table to write valid
		 * mappings.
		 * Accessing the valid area may cause page fault since FLPD
		 * cache may still caches zero_l2_table for the valid area
		 * instead of new L2 page table that have the mapping
		 * information of the valid area
		 * Thus any replacement of zero_l2_table with other valid L2
		 * page table must involve FLPD cache invalidation if the System
		 * MMU have prefetch feature and FLPD cache (version 3.3).
		 * FLPD cache invalidation is performed with TLB invalidation
		 * by VPN without blocking. It is safe to invalidate TLB without
		 * blocking because the target address of TLB invalidation is
		 * not currently mapped.
		 */
		spin_lock_irqsave(&priv->lock, flags);
		list_for_each_entry(owner, &priv->clients, client)
			sysmmu_tlb_invalidate_flpdcache(owner->dev, iova);
		spin_unlock_irqrestore(&priv->lock, flags);
	} else if (!lv1ent_page(sent)) {
		BUG();
		return ERR_PTR(-EADDRINUSE);
=======
	if (!found)
		goto finish;

	if (__exynos_sysmmu_disable(data)) {
		dev_dbg(dev, "%s: Detached IOMMU with pgtable %#lx\n",
					__func__, __pa(priv->pgtable));
		list_del_init(&data->node);

	} else {
		dev_dbg(dev, "%s: Detaching IOMMU with pgtable %#lx delayed",
					__func__, __pa(priv->pgtable));
	}

finish:
	spin_unlock_irqrestore(&priv->lock, flags);

	if (found)
		pm_runtime_put(data->sysmmu);
}

static unsigned long *alloc_lv2entry(unsigned long *sent, unsigned long iova,
					short *pgcounter)
{
	if (lv1ent_fault(sent)) {
		unsigned long *pent;

		pent = kzalloc(LV2TABLE_SIZE, GFP_ATOMIC);
		BUG_ON((unsigned long)pent & (LV2TABLE_SIZE - 1));
		if (!pent)
			return NULL;

		*sent = mk_lv1ent_page(__pa(pent));
		*pgcounter = NUM_LV2ENTRIES;
		pgtable_flush(pent, pent + NUM_LV2ENTRIES);
		pgtable_flush(sent, sent + 1);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	return page_entry(sent, iova);
}

<<<<<<< HEAD
static int lv1ent_check_page(struct exynos_iommu_domain *priv,
				sysmmu_pte_t *sent, short *pgcnt)
{
	if (lv1ent_page(sent)) {
		if (WARN_ON(*pgcnt != NUM_LV2ENTRIES))
			return -EADDRINUSE;

		kmem_cache_free(lv2table_kmem_cache, page_entry(sent, 0));

		*pgcnt = 0;

		SYSMMU_EVENT_LOG_IOMMU_FREESLPD(IOMMU_PRIV_TO_LOG(priv),
				iova_from_sent(priv->pgtable, sent));
	}

	return 0;
}

static void clear_lv1_page_table(sysmmu_pte_t *ent, int n)
{
	int i;
	for (i = 0; i < n; i++)
		ent[i] = ZERO_LV2LINK;
}

static void clear_lv2_page_table(sysmmu_pte_t *ent, int n)
{
	if (n > 0)
		memset(ent, 0, sizeof(*ent) * n);
}

static int lv1set_section(struct exynos_iommu_domain *priv,
			sysmmu_pte_t *sent, phys_addr_t paddr,
			  size_t size,  short *pgcnt)
{
	int ret;

	if (WARN_ON(!lv1ent_fault(sent) && !lv1ent_page(sent)))
		return -EADDRINUSE;

	if (size == SECT_SIZE) {
		ret = lv1ent_check_page(priv, sent, pgcnt);
		if (ret)
			return ret;
		*sent = mk_lv1ent_sect(paddr);
		pgtable_flush(sent, sent + 1);
	} else if (size == DSECT_SIZE) {
		int i;
		for (i = 0; i < SECT_PER_DSECT; i++, sent++, pgcnt++) {
			ret = lv1ent_check_page(priv, sent, pgcnt);
			if (ret) {
				clear_lv1_page_table(sent - i, i);
				return ret;
			}
			*sent = mk_lv1ent_dsect(paddr);
		}
		pgtable_flush(sent - SECT_PER_DSECT, sent);
	} else {
		int i;
		for (i = 0; i < SECT_PER_SPSECT; i++, sent++, pgcnt++) {
			ret = lv1ent_check_page(priv, sent, pgcnt);
			if (ret) {
				clear_lv1_page_table(sent - i, i);
				return ret;
			}
			*sent = mk_lv1ent_spsect(paddr);
		}
		pgtable_flush(sent - SECT_PER_SPSECT, sent);
	}
=======
static int lv1set_section(unsigned long *sent, phys_addr_t paddr, short *pgcnt)
{
	if (lv1ent_section(sent))
		return -EADDRINUSE;

	if (lv1ent_page(sent)) {
		if (*pgcnt != NUM_LV2ENTRIES)
			return -EADDRINUSE;

		kfree(page_entry(sent, 0));

		*pgcnt = 0;
	}

	*sent = mk_lv1ent_sect(paddr);

	pgtable_flush(sent, sent + 1);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	return 0;
}

<<<<<<< HEAD
static int lv2set_page(sysmmu_pte_t *pent, phys_addr_t paddr,
		       size_t size, short *pgcnt)
{
	if (size == SPAGE_SIZE) {
		if (WARN_ON(!lv2ent_fault(pent)))
=======
static int lv2set_page(unsigned long *pent, phys_addr_t paddr, size_t size,
								short *pgcnt)
{
	if (size == SPAGE_SIZE) {
		if (!lv2ent_fault(pent))
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			return -EADDRINUSE;

		*pent = mk_lv2ent_spage(paddr);
		pgtable_flush(pent, pent + 1);
		*pgcnt -= 1;
	} else { /* size == LPAGE_SIZE */
		int i;
		for (i = 0; i < SPAGES_PER_LPAGE; i++, pent++) {
<<<<<<< HEAD
			if (WARN_ON(!lv2ent_fault(pent))) {
				clear_lv2_page_table(pent - i, i);
=======
			if (!lv2ent_fault(pent)) {
				memset(pent, 0, sizeof(*pent) * i);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
				return -EADDRINUSE;
			}

			*pent = mk_lv2ent_lpage(paddr);
		}
		pgtable_flush(pent - SPAGES_PER_LPAGE, pent);
		*pgcnt -= SPAGES_PER_LPAGE;
	}

	return 0;
}

static int exynos_iommu_map(struct iommu_domain *domain, unsigned long iova,
			 phys_addr_t paddr, size_t size, int prot)
{
	struct exynos_iommu_domain *priv = domain->priv;
<<<<<<< HEAD
	sysmmu_pte_t *entry;
=======
	unsigned long *entry;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	unsigned long flags;
	int ret = -ENOMEM;

	BUG_ON(priv->pgtable == NULL);

	spin_lock_irqsave(&priv->pgtablelock, flags);

	entry = section_entry(priv->pgtable, iova);

<<<<<<< HEAD
	if (size >= SECT_SIZE) {
		int num_entry = size / SECT_SIZE;
		struct exynos_iommu_owner *owner;

		ret = lv1set_section(priv, entry, paddr, size,
					&priv->lv2entcnt[lv1ent_offset(iova)]);

		spin_lock(&priv->lock);
		list_for_each_entry(owner, &priv->clients, client) {
			int i;
			for (i = 0; i < num_entry; i++)
				sysmmu_tlb_invalidate_flpdcache(owner->dev,
						iova + i * SECT_SIZE);
		}
		spin_unlock(&priv->lock);

		SYSMMU_EVENT_LOG_IOMMU_MAP(IOMMU_PRIV_TO_LOG(priv),
				iova, iova + size, paddr / SPAGE_SIZE);
	} else {
		sysmmu_pte_t *pent;

		pent = alloc_lv2entry(priv, entry, iova,
					&priv->lv2entcnt[lv1ent_offset(iova)]);

		if (IS_ERR(pent)) {
			ret = PTR_ERR(pent);
		} else {
			ret = lv2set_page(pent, paddr, size,
					&priv->lv2entcnt[lv1ent_offset(iova)]);

			SYSMMU_EVENT_LOG_IOMMU_MAP(IOMMU_PRIV_TO_LOG(priv),
					iova, iova + size, paddr / SPAGE_SIZE);
		}
	}

	if (ret)
		pr_err("%s: Failed(%d) to map %#x bytes @ %#lx\n",
			__func__, ret, size, iova);
=======
	if (size == SECT_SIZE) {
		ret = lv1set_section(entry, paddr,
					&priv->lv2entcnt[lv1ent_offset(iova)]);
	} else {
		unsigned long *pent;

		pent = alloc_lv2entry(entry, iova,
					&priv->lv2entcnt[lv1ent_offset(iova)]);

		if (!pent)
			ret = -ENOMEM;
		else
			ret = lv2set_page(pent, paddr, size,
					&priv->lv2entcnt[lv1ent_offset(iova)]);
	}

	if (ret) {
		pr_debug("%s: Failed to map iova 0x%lx/0x%x bytes\n",
							__func__, iova, size);
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_unlock_irqrestore(&priv->pgtablelock, flags);

	return ret;
}

<<<<<<< HEAD
static void exynos_iommu_tlb_invalidate_entry(struct exynos_iommu_domain *priv,
					unsigned long iova)
{
	struct exynos_iommu_owner *owner;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);

	list_for_each_entry(owner, &priv->clients, client)
		sysmmu_tlb_invalidate_entry(owner->dev, iova, false);

	spin_unlock_irqrestore(&priv->lock, flags);
}

static size_t exynos_iommu_unmap(struct iommu_domain *domain,
					unsigned long iova, size_t size)
{
	struct exynos_iommu_domain *priv = domain->priv;
	size_t err_pgsize;
	sysmmu_pte_t *ent;
	unsigned long flags;
=======
static size_t exynos_iommu_unmap(struct iommu_domain *domain,
					       unsigned long iova, size_t size)
{
	struct exynos_iommu_domain *priv = domain->priv;
	struct sysmmu_drvdata *data;
	unsigned long flags;
	unsigned long *ent;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	BUG_ON(priv->pgtable == NULL);

	spin_lock_irqsave(&priv->pgtablelock, flags);

	ent = section_entry(priv->pgtable, iova);

<<<<<<< HEAD
	if (lv1ent_spsection(ent)) {
		if (WARN_ON(size < SPSECT_SIZE)) {
			err_pgsize = SPSECT_SIZE;
			goto err;
		}

		clear_lv1_page_table(ent, SECT_PER_SPSECT);

		pgtable_flush(ent, ent + SECT_PER_SPSECT);
		size = SPSECT_SIZE;
		goto done;
	}

	if (lv1ent_dsection(ent)) {
		if (WARN_ON(size < DSECT_SIZE)) {
			err_pgsize = DSECT_SIZE;
			goto err;
		}

		*ent = ZERO_LV2LINK;
		*(++ent) = ZERO_LV2LINK;
		pgtable_flush(ent, ent + 2);
		size = DSECT_SIZE;
		goto done;
	}

	if (lv1ent_section(ent)) {
		if (WARN_ON(size < SECT_SIZE)) {
			err_pgsize = SECT_SIZE;
			goto err;
		}

		*ent = ZERO_LV2LINK;
=======
	if (lv1ent_section(ent)) {
		BUG_ON(size < SECT_SIZE);

		*ent = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		pgtable_flush(ent, ent + 1);
		size = SECT_SIZE;
		goto done;
	}

	if (unlikely(lv1ent_fault(ent))) {
		if (size > SECT_SIZE)
			size = SECT_SIZE;
		goto done;
	}

	/* lv1ent_page(sent) == true here */

	ent = page_entry(ent, iova);

	if (unlikely(lv2ent_fault(ent))) {
		size = SPAGE_SIZE;
		goto done;
	}

	if (lv2ent_small(ent)) {
		*ent = 0;
		size = SPAGE_SIZE;
<<<<<<< HEAD
		pgtable_flush(ent, ent + 1);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		priv->lv2entcnt[lv1ent_offset(iova)] += 1;
		goto done;
	}

	/* lv1ent_large(ent) == true here */
<<<<<<< HEAD
	if (WARN_ON(size < LPAGE_SIZE)) {
		err_pgsize = LPAGE_SIZE;
		goto err;
	}

	clear_lv2_page_table(ent, SPAGES_PER_LPAGE);
	pgtable_flush(ent, ent + SPAGES_PER_LPAGE);
=======
	BUG_ON(size < LPAGE_SIZE);

	memset(ent, 0, sizeof(*ent) * SPAGES_PER_LPAGE);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	size = LPAGE_SIZE;
	priv->lv2entcnt[lv1ent_offset(iova)] += SPAGES_PER_LPAGE;
done:
	spin_unlock_irqrestore(&priv->pgtablelock, flags);

<<<<<<< HEAD
	SYSMMU_EVENT_LOG_IOMMU_UNMAP(IOMMU_PRIV_TO_LOG(priv),
						iova, iova + size);

	exynos_iommu_tlb_invalidate_entry(priv, iova);

	/* TLB invalidation is performed by IOVMM */
	return size;
err:
	spin_unlock_irqrestore(&priv->pgtablelock, flags);

	pr_err("%s: Failed: size(%#lx) @ %#x is smaller than page size %#x\n",
		__func__, iova, size, err_pgsize);

	return 0;
}

static phys_addr_t exynos_iommu_iova_to_phys(struct iommu_domain *domain,
					     dma_addr_t iova)
{
	struct exynos_iommu_domain *priv = domain->priv;
	unsigned long flags;
	sysmmu_pte_t *entry;
=======
	spin_lock_irqsave(&priv->lock, flags);
	list_for_each_entry(data, &priv->clients, node)
		sysmmu_tlb_invalidate_entry(data->dev, iova);
	spin_unlock_irqrestore(&priv->lock, flags);


	return size;
}

static phys_addr_t exynos_iommu_iova_to_phys(struct iommu_domain *domain,
					  dma_addr_t iova)
{
	struct exynos_iommu_domain *priv = domain->priv;
	unsigned long *entry;
	unsigned long flags;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	phys_addr_t phys = 0;

	spin_lock_irqsave(&priv->pgtablelock, flags);

	entry = section_entry(priv->pgtable, iova);

<<<<<<< HEAD
	if (lv1ent_spsection(entry)) {
		phys = spsection_phys(entry) + spsection_offs(iova);
	} else if (lv1ent_dsection(entry)) {
		phys = dsection_phys(entry) + dsection_offs(iova);
	} else if (lv1ent_section(entry)) {
=======
	if (lv1ent_section(entry)) {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		phys = section_phys(entry) + section_offs(iova);
	} else if (lv1ent_page(entry)) {
		entry = page_entry(entry, iova);

		if (lv2ent_large(entry))
			phys = lpage_phys(entry) + lpage_offs(iova);
		else if (lv2ent_small(entry))
			phys = spage_phys(entry) + spage_offs(iova);
	}

	spin_unlock_irqrestore(&priv->pgtablelock, flags);

	return phys;
}

static struct iommu_ops exynos_iommu_ops = {
	.domain_init = &exynos_iommu_domain_init,
	.domain_destroy = &exynos_iommu_domain_destroy,
	.attach_dev = &exynos_iommu_attach_device,
	.detach_dev = &exynos_iommu_detach_device,
	.map = &exynos_iommu_map,
	.unmap = &exynos_iommu_unmap,
	.iova_to_phys = &exynos_iommu_iova_to_phys,
<<<<<<< HEAD
	.pgsize_bitmap = PGSIZE_BITMAP,
};

static int __sysmmu_unmap_user_pages(struct device *dev,
					struct mm_struct *mm,
					unsigned long vaddr, size_t size)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct exynos_iovmm *vmm = owner->vmm_data;
	struct iommu_domain *domain = vmm->domain;
	struct exynos_iommu_domain *priv = domain->priv;
	struct vm_area_struct *vma;
	unsigned long start = vaddr & PAGE_MASK;
	unsigned long end = PAGE_ALIGN(vaddr + size);
	bool is_pfnmap;
	sysmmu_pte_t *sent, *pent;
	int ret = 0;

	down_read(&mm->mmap_sem);

	BUG_ON((vaddr + size) < vaddr);
	/*
	 * Assumes that the VMA is safe.
	 * The caller must check the range of address space before calling this.
	 */
	vma = find_vma(mm, vaddr);
	if (!vma) {
		pr_err("%s: vma is null\n", __func__);
		ret = -EINVAL;
		goto out_unmap;
	}

	if (vma->vm_end < (vaddr + size)) {
		pr_err("%s: vma overflow: %#lx--%#lx, vaddr: %#lx, size: %zd\n",
			__func__, vma->vm_start, vma->vm_end, vaddr, size);
		ret = -EINVAL;
		goto out_unmap;
	}

	is_pfnmap = vma->vm_flags & VM_PFNMAP;

	TRACE_LOG_DEV(dev, "%s: Unmap starts @ %#x@%#lx\n",
			__func__, size, start);

	do {
		sysmmu_pte_t *pent_first;
		int i;

		sent = section_entry(priv->pgtable, start);
		if (lv1ent_fault(sent)) {
			ret = -EFAULT;
			goto out_unmap;
		}

		pent = page_entry(sent, start);
		if (lv2ent_fault(pent)) {
			ret = -EFAULT;
			goto out_unmap;
		}

		pent_first = pent;
		do {
			i = lv2ent_offset(start);

			if (!lv2ent_fault(pent) && !is_pfnmap)
				put_page(phys_to_page(spage_phys(pent)));

			*pent = 0;
			start += PAGE_SIZE;
		} while (pent++, (start != end) && (i < (NUM_LV2ENTRIES - 1)));

		pgtable_flush(pent_first, pent);
	} while (start != end);


	TRACE_LOG_DEV(dev, "%s: unmap done @ %#lx\n", __func__, start);

out_unmap:
	up_read(&mm->mmap_sem);

	if (ret) {
		pr_debug("%s: Ignoring unmapping for %#lx ~ %#lx\n",
					__func__, start, end);
	}

	return ret;
}

static sysmmu_pte_t *alloc_lv2entry_fast(struct exynos_iommu_domain *priv,
		sysmmu_pte_t *sent, unsigned long iova)
{
	if (lv1ent_fault(sent)) {
		sysmmu_pte_t *pent;

		pent = kmem_cache_zalloc(lv2table_kmem_cache, GFP_ATOMIC);
		BUG_ON((unsigned long)pent & (LV2TABLE_SIZE - 1));
		if (!pent)
			return ERR_PTR(-ENOMEM);

		*sent = mk_lv1ent_page(__pa(pent));
		kmemleak_ignore(pent);
		pgtable_flush(sent, sent + 1);
	} else if (WARN_ON(!lv1ent_page(sent))) {
		return ERR_PTR(-EADDRINUSE);
	}

	return page_entry(sent, iova);
}

int exynos_sysmmu_map_user_pages(struct device *dev,
					struct mm_struct *mm,
					unsigned long vaddr,
					size_t size, int write)
{
	struct exynos_iommu_owner *owner = dev->archdata.iommu;
	struct exynos_iovmm *vmm = owner->vmm_data;
	struct iommu_domain *domain = vmm->domain;
	struct exynos_iommu_domain *priv = domain->priv;
	struct vm_area_struct *vma;
	unsigned long start, end;
	unsigned long pgd_next;
	int ret = -EINVAL;
	bool is_pfnmap;
	pgd_t *pgd;

	if (WARN_ON(size == 0))
		return 0;

	down_read(&mm->mmap_sem);

	/*
	 * Assumes that the VMA is safe.
	 * The caller must check the range of address space before calling this.
	 */
	vma = find_vma(mm, vaddr);
	if (!vma) {
		pr_err("%s: vma is null\n", __func__);
		up_read(&mm->mmap_sem);
		return -EINVAL;
	}

	if (vma->vm_end < (vaddr + size)) {
		pr_err("%s: vma overflow: %#lx--%#lx, vaddr: %#lx, size: %zd\n",
			__func__, vma->vm_start, vma->vm_end, vaddr, size);
		up_read(&mm->mmap_sem);
		return -EINVAL;
	}

	is_pfnmap = vma->vm_flags & VM_PFNMAP;

	start = vaddr & PAGE_MASK;
	end = PAGE_ALIGN(vaddr + size);

	TRACE_LOG_DEV(dev, "%s: map @ %#lx--%#lx, %d bytes, vm_flags: %#lx\n",
			__func__, start, end, size, vma->vm_flags);

	pgd = pgd_offset(mm, start);
	do {
		unsigned long pmd_next;
		pmd_t *pmd;

		if (pgd_none_or_clear_bad(pgd)) {
			ret = -EBADR;
			goto out_unmap;
		}

		pgd_next = pgd_addr_end(start, end);
		pmd = pmd_offset((pud_t *)pgd, start);

		do {
			pte_t *pte, *pte_first;
			sysmmu_pte_t *pent, *pent_first;
			sysmmu_pte_t *sent;
			spinlock_t *ptl;

			if (pmd_none_or_clear_bad(pmd)) {
				ret = -EBADR;
				goto out_unmap;
			}

			pmd_next = pmd_addr_end(start, pgd_next);
			pmd_next = min(ALIGN(start + 1, SECT_SIZE), pmd_next);

			pte = pte_offset_map(pmd, start);
			pte_first = pte;

			sent = section_entry(priv->pgtable, start);
			pent = alloc_lv2entry_fast(priv, sent, start);
			if (IS_ERR(pent)) {
				ret = PTR_ERR(pent); /* ENOMEM or EADDRINUSE */
				goto out_unmap;
			}

			pent_first = pent;
			ptl = pte_lockptr(mm, pmd);

			spin_lock(ptl);
			do {
				if (!pte_present(*pte) ||
					(write && !pte_write(*pte))) {
					spin_unlock(ptl);
					ret = handle_pte_fault(mm,
						vma, start, pte, pmd,
						write ? FAULT_FLAG_WRITE : 0);
					if (IS_ERR_VALUE(ret)) {
						ret = -EIO;
						goto out_unmap;
					}
					spin_lock(ptl);
				}

				if (!pte_present(*pte) ||
					(write && !pte_write(*pte))) {
					ret = -EPERM;
					spin_unlock(ptl);
					goto out_unmap;
				}

				if (!is_pfnmap)
					get_page(pte_page(*pte));
				*pent = mk_lv2ent_spage(__pfn_to_phys(
							pte_pfn(*pte)));
			} while (pte++, pent++,
				start += PAGE_SIZE, start < pmd_next);

			pte_unmap(pte_first);
			pgtable_flush(pent_first, pent);
			spin_unlock(ptl);
		} while (pmd++, start = pmd_next, start != pgd_next);

	} while (pgd++, start = pgd_next, start != end);

	ret = 0;
out_unmap:
	up_read(&mm->mmap_sem);

	if (ret) {
		pr_debug("%s: Ignoring mapping for %#lx ~ %#lx (%d)\n",
					__func__, start, end, ret);
		__sysmmu_unmap_user_pages(dev, mm, vaddr,
					start - (vaddr & PAGE_MASK));
	}

	return ret;
}

int exynos_sysmmu_unmap_user_pages(struct device *dev,
					struct mm_struct *mm,
					unsigned long vaddr, size_t size)
{
	if (WARN_ON(size == 0))
		return 0;

	return __sysmmu_unmap_user_pages(dev, mm, vaddr, size);
}

static int __init exynos_iommu_init(void)
{
	struct page *page;
	int ret = -ENOMEM;

	lv2table_kmem_cache = kmem_cache_create("exynos-iommu-lv2table",
		LV2TABLE_SIZE, LV2TABLE_SIZE, 0, NULL);
	if (!lv2table_kmem_cache) {
		pr_err("%s: failed to create kmem cache\n", __func__);
		return -ENOMEM;
	}

	page = alloc_page(GFP_KERNEL | __GFP_ZERO);
	if (!page) {
		pr_err("%s: failed to allocate fault page\n", __func__);
		goto err_fault_page;
	}
	fault_page = page_to_phys(page);

	ret = bus_set_iommu(&platform_bus_type, &exynos_iommu_ops);
	if (ret) {
		pr_err("%s: Failed to register IOMMU ops\n", __func__);
		goto err_set_iommu;
	}

	zero_lv2_table = kmem_cache_zalloc(lv2table_kmem_cache, GFP_KERNEL);
	if (zero_lv2_table == NULL) {
		pr_err("%s: Failed to allocate zero level2 page table\n",
			__func__);
		ret = -ENOMEM;
		goto err_zero_lv2;
	}

	exynos_sysmmu_debugfs_root = debugfs_create_dir("sysmmu", NULL);
	if (!exynos_sysmmu_debugfs_root)
		pr_err("%s: Failed to create debugfs entry\n", __func__);

	ret = platform_driver_register(&exynos_sysmmu_driver);
	if (ret) {
		pr_err("%s: Failed to register System MMU driver.\n", __func__);
		goto err_driver_register;
	}

	return 0;
err_driver_register:
	kmem_cache_free(lv2table_kmem_cache, zero_lv2_table);
err_zero_lv2:
	bus_set_iommu(&platform_bus_type, NULL);
err_set_iommu:
	__free_page(page);
err_fault_page:
	kmem_cache_destroy(lv2table_kmem_cache);
	return ret;
}
arch_initcall(exynos_iommu_init);

#ifdef CONFIG_PM_SLEEP
static int sysmmu_pm_genpd_suspend(struct device *dev)
{
	struct sysmmu_list_data *list;
	int ret;

	TRACE_LOG("%s(%s) ----->\n", __func__, dev_name(dev));

	ret = pm_generic_suspend(dev);
	if (ret) {
		TRACE_LOG("<----- %s(%s) Failed\n", __func__, dev_name(dev));
		return ret;
	}

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);
		unsigned long flags;
		TRACE_LOG("Suspending %s...\n", dev_name(drvdata->sysmmu));
		spin_lock_irqsave(&drvdata->lock, flags);
		if (!drvdata->suspended && is_sysmmu_active(drvdata) &&
			(!pm_runtime_enabled(dev) || drvdata->runtime_active))
			__sysmmu_disable_nocount(drvdata);
		drvdata->suspended = true;
		spin_unlock_irqrestore(&drvdata->lock, flags);
	}

	TRACE_LOG("<----- %s(%s)\n", __func__, dev_name(dev));

	return 0;
}

static int sysmmu_pm_genpd_resume(struct device *dev)
{
	struct sysmmu_list_data *list;
	int ret;

	TRACE_LOG("%s(%s) ----->\n", __func__, dev_name(dev));

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);
		unsigned long flags;
		spin_lock_irqsave(&drvdata->lock, flags);
		if (drvdata->suspended && is_sysmmu_active(drvdata) &&
			(!pm_runtime_enabled(dev) || drvdata->runtime_active))
			__sysmmu_enable_nocount(drvdata);
		drvdata->suspended = false;
		spin_unlock_irqrestore(&drvdata->lock, flags);
	}

	ret = pm_generic_resume(dev);

	TRACE_LOG("<----- %s(%s) OK\n", __func__, dev_name(dev));

	return ret;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static void sysmmu_restore_state(struct device *dev)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *data = dev_get_drvdata(list->sysmmu);
		unsigned long flags;

		TRACE_LOG("%s(%s)\n", __func__, dev_name(data->sysmmu));

		SYSMMU_EVENT_LOG_POWERON(SYSMMU_DRVDATA_TO_LOG(data));

		spin_lock_irqsave(&data->lock, flags);
		if (!data->runtime_active && is_sysmmu_active(data))
			__sysmmu_enable_nocount(data);
		data->runtime_active = true;
		spin_unlock_irqrestore(&data->lock, flags);
	}
}

static void sysmmu_save_state(struct device *dev)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *data = dev_get_drvdata(list->sysmmu);
		unsigned long flags;

		TRACE_LOG("%s(%s)\n", __func__, dev_name(data->sysmmu));

		SYSMMU_EVENT_LOG_POWEROFF(SYSMMU_DRVDATA_TO_LOG(data));

		spin_lock_irqsave(&data->lock, flags);
		if (data->runtime_active && is_sysmmu_active(data))
			__sysmmu_disable_nocount(data);
		data->runtime_active = false;
		spin_unlock_irqrestore(&data->lock, flags);
	}
}

static int sysmmu_pm_genpd_save_state(struct device *dev)
{
	int (*cb)(struct device *__dev);
	int ret = 0;

	TRACE_LOG("%s(%s) ----->\n", __func__, dev_name(dev));

	if (dev->type && dev->type->pm)
		cb = dev->type->pm->runtime_suspend;
	else if (dev->class && dev->class->pm)
		cb = dev->class->pm->runtime_suspend;
	else if (dev->bus && dev->bus->pm)
		cb = dev->bus->pm->runtime_suspend;
	else
		cb = NULL;

	if (!cb && dev->driver && dev->driver->pm)
		cb = dev->driver->pm->runtime_suspend;

	if (cb)
		ret = cb(dev);

	if (ret == 0)
		sysmmu_save_state(dev);

	TRACE_LOG("<----- %s(%s) (cb = %pS) %s\n", __func__, dev_name(dev),
			cb, ret ? "Failed" : "OK");

	return ret;
}

static int sysmmu_pm_genpd_restore_state(struct device *dev)
{
	int (*cb)(struct device *__dev);
	int ret = 0;

	TRACE_LOG("%s(%s) ----->\n", __func__, dev_name(dev));

	if (dev->type && dev->type->pm)
		cb = dev->type->pm->runtime_resume;
	else if (dev->class && dev->class->pm)
		cb = dev->class->pm->runtime_resume;
	else if (dev->bus && dev->bus->pm)
		cb = dev->bus->pm->runtime_resume;
	else
		cb = NULL;

	if (!cb && dev->driver && dev->driver->pm)
		cb = dev->driver->pm->runtime_resume;

	sysmmu_restore_state(dev);

	if (cb)
		ret = cb(dev);

	if (ret)
		sysmmu_save_state(dev);

	TRACE_LOG("<----- %s(%s) (cb = %pS) %s\n", __func__, dev_name(dev),
			cb, ret ? "Failed" : "OK");

	return ret;
}
#endif

#ifdef CONFIG_PM_GENERIC_DOMAINS
static struct gpd_dev_ops sysmmu_devpm_ops = {
#ifdef CONFIG_PM_RUNTIME
	.save_state = &sysmmu_pm_genpd_save_state,
	.restore_state = &sysmmu_pm_genpd_restore_state,
#endif
#ifdef CONFIG_PM_SLEEP
	.suspend = &sysmmu_pm_genpd_suspend,
	.resume = &sysmmu_pm_genpd_resume,
#endif
};
#endif /* CONFIG_PM_GENERIC_DOMAINS */

#ifdef CONFIG_PM_GENERIC_DOMAINS
static int sysmmu_hook_driver_register(struct notifier_block *nb,
					unsigned long val,
					void *p)
{
	struct device *dev = p;

	/*
	 * No System MMU assigned. See exynos_sysmmu_probe().
	 */
	if (dev->archdata.iommu == NULL)
		return 0;

	switch (val) {
	case BUS_NOTIFY_BIND_DRIVER:
	{
		if (dev->pm_domain) {
			int ret = pm_genpd_add_callbacks(
					dev, &sysmmu_devpm_ops, NULL);
			if (ret && (ret != -ENOSYS)) {
				dev_err(dev,
				"Failed to register 'dev_pm_ops' for iommu\n");
				return ret;
			}

			dev_info(dev, "exynos-iommu gpd_dev_ops inserted!\n");
		}

		break;
	}
	case BUS_NOTIFY_BOUND_DRIVER:
	{
		struct sysmmu_list_data *list;

		if (pm_runtime_enabled(dev) && dev->pm_domain)
			break;

		for_each_sysmmu_list(dev, list) {
			struct sysmmu_drvdata *data =
						dev_get_drvdata(list->sysmmu);
			unsigned long flags;
			spin_lock_irqsave(&data->lock, flags);
			if (is_sysmmu_active(data) && !data->runtime_active)
				__sysmmu_enable_nocount(data);
			data->runtime_active = true;
			pm_runtime_disable(data->sysmmu);
			spin_unlock_irqrestore(&data->lock, flags);
		}

		break;
	}
	case BUS_NOTIFY_UNBOUND_DRIVER:
	{
		struct exynos_iommu_owner *owner = dev->archdata.iommu;
		WARN_ON(!list_empty(&owner->client));
		__pm_genpd_remove_callbacks(dev, false);
		dev_info(dev, "exynos-iommu gpd_dev_ops removed!\n");
		break;
	}
	} /* switch (val) */

	return 0;
}

static struct notifier_block sysmmu_notifier = {
	.notifier_call = &sysmmu_hook_driver_register,
};

static int __init exynos_iommu_prepare(void)
{
	return bus_register_notifier(&platform_bus_type, &sysmmu_notifier);
}
arch_initcall_sync(exynos_iommu_prepare);
#endif

static void sysmmu_dump_lv2_page_table(unsigned int lv1idx, sysmmu_pte_t *base)
{
	unsigned int i;
	for (i = 0; i < NUM_LV2ENTRIES; i += 4) {
		if (!base[i] && !base[i + 1] && !base[i + 2] && !base[i + 3])
			continue;
		pr_info("    LV2[%04d][%03d] %08x %08x %08x %08x\n",
			lv1idx, i,
			base[i], base[i + 1], base[i + 2], base[i + 3]);
	}
}

static void sysmmu_dump_page_table(sysmmu_pte_t *base)
{
	unsigned int i;

	pr_info("---- System MMU Page Table @ %#010x (ZeroLv2Desc: %#x) ----\n",
		virt_to_phys(base), ZERO_LV2LINK);

	for (i = 0; i < NUM_LV1ENTRIES; i += 4) {
		unsigned int j;
		if ((base[i] == ZERO_LV2LINK) &&
			(base[i + 1] == ZERO_LV2LINK) &&
			(base[i + 2] == ZERO_LV2LINK) &&
			(base[i + 3] == ZERO_LV2LINK))
			continue;
		pr_info("LV1[%04d] %08x %08x %08x %08x\n",
			i, base[i], base[i + 1], base[i + 2], base[i + 3]);

		for (j = 0; j < 4; j++)
			if (lv1ent_page(&base[i + j]))
				sysmmu_dump_lv2_page_table(i + j,
						page_entry(&base[i + j], 0));
	}
}

void exynos_sysmmu_show_status(struct device *dev)
{
	struct sysmmu_list_data *list;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			dev_info(drvdata->sysmmu,
				"%s: System MMU is not active\n", __func__);
			continue;
		}

		pr_info("DUMPING SYSTEM MMU: %s\n", dev_name(drvdata->sysmmu));

		__master_clk_enable(drvdata);
		if (sysmmu_block(drvdata->sfrbase))
			dump_sysmmu_tlb_pb(drvdata->sfrbase);
		else
			pr_err("!!Failed to block Sytem MMU!\n");
		sysmmu_unblock(drvdata->sfrbase);

		__master_clk_disable(drvdata);

		sysmmu_dump_page_table(phys_to_virt(drvdata->pgtable));
	}
}

void exynos_sysmmu_show_ppc_event(struct device *dev)
{
	struct sysmmu_list_data *list;
	unsigned long flags;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock_irqsave(&drvdata->lock, flags);
		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			dev_info(drvdata->sysmmu,
				"%s: System MMU is not active\n", __func__);
			spin_unlock_irqrestore(&drvdata->lock, flags);
			continue;
		}

		__master_clk_enable(drvdata);
		if (sysmmu_block(drvdata->sfrbase))
			dump_sysmmu_ppc_cnt(drvdata);
		else
			pr_err("!!Failed to block Sytem MMU!\n");
		sysmmu_unblock(drvdata->sfrbase);
		__master_clk_disable(drvdata);
		spin_unlock_irqrestore(&drvdata->lock, flags);
	}
}

void exynos_sysmmu_clear_ppc_event(struct device *dev)
{
	struct sysmmu_list_data *list;
	unsigned long flags;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock_irqsave(&drvdata->lock, flags);
		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			dev_info(drvdata->sysmmu,
				"%s: System MMU is not active\n", __func__);
			spin_unlock_irqrestore(&drvdata->lock, flags);
			continue;
		}

		__master_clk_enable(drvdata);
		if (sysmmu_block(drvdata->sfrbase)) {
			dump_sysmmu_ppc_cnt(drvdata);
			__raw_writel(0x2, drvdata->sfrbase + REG_PPC_PMNC);
			__raw_writel(0, drvdata->sfrbase + REG_PPC_CNTENS);
			__raw_writel(0, drvdata->sfrbase + REG_PPC_INTENS);
			drvdata->event_cnt = 0;
		} else
			pr_err("!!Failed to block Sytem MMU!\n");
		sysmmu_unblock(drvdata->sfrbase);
		__master_clk_disable(drvdata);

		spin_unlock_irqrestore(&drvdata->lock, flags);
	}
}

int exynos_sysmmu_set_ppc_event(struct device *dev, int event)
{
	struct sysmmu_list_data *list;
	unsigned long flags;
	int ret = 0;

	for_each_sysmmu_list(dev, list) {
		struct sysmmu_drvdata *drvdata = dev_get_drvdata(list->sysmmu);

		spin_lock_irqsave(&drvdata->lock, flags);
		if (!is_sysmmu_active(drvdata) || !drvdata->runtime_active) {
			dev_info(drvdata->sysmmu,
				"%s: System MMU is not active\n", __func__);
			spin_unlock_irqrestore(&drvdata->lock, flags);
			continue;
		}

		__master_clk_enable(drvdata);
		if (sysmmu_block(drvdata->sfrbase)) {
			if (drvdata->event_cnt < MAX_NUM_PPC) {
				ret = sysmmu_set_ppc_event(drvdata, event);
				if (ret)
					pr_err("Not supported Event ID (%d)",
						event);
				else
					drvdata->event_cnt++;
			}
		} else
			pr_err("!!Failed to block Sytem MMU!\n");
		sysmmu_unblock(drvdata->sfrbase);
		__master_clk_disable(drvdata);

		spin_unlock_irqrestore(&drvdata->lock, flags);
	}

	return ret;
}
=======
	.pgsize_bitmap = SECT_SIZE | LPAGE_SIZE | SPAGE_SIZE,
};

static int __init exynos_iommu_init(void)
{
	int ret;

	ret = platform_driver_register(&exynos_sysmmu_driver);

	if (ret == 0)
		bus_set_iommu(&platform_bus_type, &exynos_iommu_ops);

	return ret;
}
subsys_initcall(exynos_iommu_init);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
