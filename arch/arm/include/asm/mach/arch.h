/*
 *  arch/arm/include/asm/mach/arch.h
 *
 *  Copyright (C) 2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASSEMBLY__
<<<<<<< HEAD

struct tag;
struct meminfo;
=======
#include <linux/reboot.h>

struct tag;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
struct pt_regs;
struct smp_operations;
#ifdef CONFIG_SMP
#define smp_ops(ops) (&(ops))
#else
#define smp_ops(ops) (struct smp_operations *)NULL
#endif

struct machine_desc {
	unsigned int		nr;		/* architecture number	*/
	const char		*name;		/* architecture name	*/
	unsigned long		atag_offset;	/* tagged list (relative) */
	const char *const 	*dt_compat;	/* array of device tree
						 * 'compatible' strings	*/

	unsigned int		nr_irqs;	/* number of IRQs */

#ifdef CONFIG_ZONE_DMA
	unsigned long		dma_zone_size;	/* size of DMA-able area */
#endif

	unsigned int		video_start;	/* start of video RAM	*/
	unsigned int		video_end;	/* end of video RAM	*/

	unsigned char		reserve_lp0 :1;	/* never has lp0	*/
	unsigned char		reserve_lp1 :1;	/* never has lp1	*/
	unsigned char		reserve_lp2 :1;	/* never has lp2	*/
<<<<<<< HEAD
	char			restart_mode;	/* default restart mode	*/
	struct smp_operations	*smp;		/* SMP operations	*/
	void			(*fixup)(struct tag *, char **,
					 struct meminfo *);
=======
	enum reboot_mode	reboot_mode;	/* default restart mode	*/
	struct smp_operations	*smp;		/* SMP operations	*/
	void			(*fixup)(struct tag *, char **);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	void			(*reserve)(void);/* reserve mem blocks	*/
	void			(*map_io)(void);/* IO mapping function	*/
	void			(*init_early)(void);
	void			(*init_irq)(void);
	void			(*init_time)(void);
	void			(*init_machine)(void);
	void			(*init_late)(void);
#ifdef CONFIG_MULTI_IRQ_HANDLER
	void			(*handle_irq)(struct pt_regs *);
#endif
<<<<<<< HEAD
	void			(*restart)(char, const char *);
=======
	void			(*restart)(enum reboot_mode, const char *);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
};

/*
 * Current machine - only accessible during boot.
 */
<<<<<<< HEAD
extern struct machine_desc *machine_desc;
=======
extern const struct machine_desc *machine_desc;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

/*
 * Machine type table - also only accessible during boot
 */
<<<<<<< HEAD
extern struct machine_desc __arch_info_begin[], __arch_info_end[];
=======
extern const struct machine_desc __arch_info_begin[], __arch_info_end[];
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#define for_each_machine_desc(p)			\
	for (p = __arch_info_begin; p < __arch_info_end; p++)

/*
 * Set of macros to define architecture features.  This is built into
 * a table by the linker.
 */
#define MACHINE_START(_type,_name)			\
static const struct machine_desc __mach_desc_##_type	\
 __used							\
 __attribute__((__section__(".arch.info.init"))) = {	\
	.nr		= MACH_TYPE_##_type,		\
	.name		= _name,

#define MACHINE_END				\
};

#define DT_MACHINE_START(_name, _namestr)		\
static const struct machine_desc __mach_desc_##_name	\
 __used							\
 __attribute__((__section__(".arch.info.init"))) = {	\
	.nr		= ~0,				\
	.name		= _namestr,

#endif
