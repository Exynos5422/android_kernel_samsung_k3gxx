/* linux arch/arm/mach-exynos4/hotplug.c
 *
 *  Cloned from linux/arch/arm/mach-realview/hotplug.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/cp15.h>
#include <asm/smp_plat.h>

#include <mach/regs-pmu.h>
<<<<<<< HEAD
#include <mach/pmu.h>
#include <mach/smc.h>
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <plat/cpu.h>

#include "common.h"

<<<<<<< HEAD
#define L2_CCI_OFF (1 << 1)
#define CHECK_CCI_SNOOP (1 << 7)

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static inline void cpu_enter_lowpower_a9(void)
{
	unsigned int v;

	asm volatile(
	"	mcr	p15, 0, %1, c7, c5, 0\n"
	"	mcr	p15, 0, %1, c7, c10, 4\n"
	/*
	 * Turn off coherency
	 */
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	bic	%0, %0, %3\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	bic	%0, %0, %2\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	  : "=&r" (v)
	  : "r" (0), "Ir" (CR_C), "Ir" (0x40)
	  : "cc");
}

static inline void cpu_enter_lowpower_a15(void)
{
	unsigned int v;

	asm volatile(
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	bic	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	  : "=&r" (v)
	  : "Ir" (CR_C)
	  : "cc");

	flush_cache_louis();

	asm volatile(
	/*
	* Turn off coherency
	*/
<<<<<<< HEAD
	"       clrex\n"
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	bic	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	: "=&r" (v)
	: "Ir" (0x40)
	: "cc");

	isb();
	dsb();
}

static inline void cpu_leave_lowpower(void)
{
	unsigned int v;

	asm volatile(
	"mrc	p15, 0, %0, c1, c0, 0\n"
	"	orr	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	orr	%0, %0, %2\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	  : "=&r" (v)
	  : "Ir" (CR_C), "Ir" (0x40)
	  : "cc");
}

<<<<<<< HEAD
void exynos_power_down_cpu(unsigned int cpu)
{
	struct cpumask mask;
	int type = !cpumask_and(&mask, cpu_online_mask, cpu_coregroup_mask(cpu));

	set_boot_flag(cpu, HOTPLUG);
	exynos_cpu.power_down(cpu);

#ifdef CONFIG_EXYNOS_CLUSTER_POWER_DOWN
	if (soc_is_exynos5422()) {
		u32 cluster_id = MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 1);
		if (type)
			__raw_writel(0, EXYNOS_COMMON_CONFIGURATION(cluster_id));
	}
#endif
#ifdef CONFIG_ARM_TRUSTZONE
	exynos_smc(SMC_CMD_SHUTDOWN,
		   OP_TYPE_CLUSTER & type,
		   SMC_POWERSTATE_IDLE,
		   0);
#endif
}

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
	for (;;) {

<<<<<<< HEAD
		/* make secondary cpus to be turned off at next WFI command */
		exynos_power_down_cpu(cpu);
#ifndef CONFIG_ARM_TRUSTZONE
=======
		/* make cpu1 to be turned off at next WFI command */
		if (cpu == 1)
			__raw_writel(0, S5P_ARM_CORE1_CONFIGURATION);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		/*
		 * here's the WFI
		 */
		asm(".word	0xe320f003\n"
		    :
		    :
		    : "memory", "cc");
<<<<<<< HEAD
#endif
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;
	}
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void __ref exynos_cpu_die(unsigned int cpu)
{
	int spurious = 0;
<<<<<<< HEAD
#ifndef CONFIG_ARM_TRUSTZONE
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	int primary_part = 0;

	/*
	 * we're ready for shutdown now, so do it.
	 * Exynos4 is A9 based while Exynos5 is A15; check the CPU part
	 * number by reading the Main ID register and then perform the
	 * appropriate sequence for entering low power.
	 */
	asm("mrc p15, 0, %0, c0, c0, 0" : "=r"(primary_part) : : "cc");
<<<<<<< HEAD
	primary_part &= 0xfff0;
	if ((primary_part == 0xc0f0) || (primary_part == 0xc070))
		cpu_enter_lowpower_a15();
	else
		cpu_enter_lowpower_a9();
#endif
=======
	if ((primary_part & 0xfff0) == 0xc0f0)
		cpu_enter_lowpower_a15();
	else
		cpu_enter_lowpower_a9();
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	platform_do_lowpower(cpu, &spurious);

	/*
	 * bring this CPU back into the world of cache
	 * coherency, and then restore interrupts
	 */
	cpu_leave_lowpower();

	if (spurious)
		pr_warn("CPU%u: %u spurious wakeup calls\n", cpu, spurious);
}
