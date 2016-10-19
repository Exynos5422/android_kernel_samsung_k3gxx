/*
 *  arch/arm/include/asm/prom.h
 *
 *  Copyright (C) 2009 Canonical Ltd. <jeremy.kerr@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef __ASMARM_PROM_H
#define __ASMARM_PROM_H

#define HAVE_ARCH_DEVTREE_FIXUPS

#ifdef CONFIG_OF

<<<<<<< HEAD
extern struct machine_desc *setup_machine_fdt(unsigned int dt_phys);
extern void arm_dt_memblock_reserve(void);
=======
extern const struct machine_desc *setup_machine_fdt(unsigned int dt_phys);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
extern void __init arm_dt_init_cpu_maps(void);

#else /* CONFIG_OF */

<<<<<<< HEAD
static inline struct machine_desc *setup_machine_fdt(unsigned int dt_phys)
=======
static inline const struct machine_desc *setup_machine_fdt(unsigned int dt_phys)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	return NULL;
}

<<<<<<< HEAD
static inline void arm_dt_memblock_reserve(void) { }
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static inline void arm_dt_init_cpu_maps(void) { }

#endif /* CONFIG_OF */
#endif /* ASMARM_PROM_H */
