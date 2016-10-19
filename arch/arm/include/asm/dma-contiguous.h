#ifndef ASMARM_DMA_CONTIGUOUS_H
#define ASMARM_DMA_CONTIGUOUS_H

#ifdef __KERNEL__
#ifdef CONFIG_CMA

#include <linux/types.h>
#include <asm-generic/dma-contiguous.h>

void dma_contiguous_early_fixup(phys_addr_t base, unsigned long size);
<<<<<<< HEAD
=======
void __init dma_contiguous_early_removal_fixup(void);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#endif
#endif

#endif
