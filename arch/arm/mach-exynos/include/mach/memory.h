/* linux/arch/arm/mach-exynos4/include/mach/memory.h
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS4 - Memory definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H __FILE__

<<<<<<< HEAD
#if defined(CONFIG_SOC_EXYNOS5430) || defined(CONFIG_SOC_EXYNOS5433)
#define PLAT_PHYS_OFFSET		UL(0x20000000)
#else
#define PLAT_PHYS_OFFSET		UL(0x40000000)
#endif
=======
#define PLAT_PHYS_OFFSET		UL(0x40000000)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

/* Maximum of 256MiB in one bank */
#define MAX_PHYSMEM_BITS	32
#define SECTION_SIZE_BITS	28

<<<<<<< HEAD
#define ARCH_HAS_SG_CHAIN

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#endif /* __ASM_ARCH_MEMORY_H */
