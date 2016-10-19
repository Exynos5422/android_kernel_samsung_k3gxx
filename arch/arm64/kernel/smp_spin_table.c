/*
 * Spin Table SMP initialisation
 *
 * Copyright (C) 2013 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

<<<<<<< HEAD
=======
#include <linux/delay.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <linux/init.h>
#include <linux/of.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
<<<<<<< HEAD

static phys_addr_t cpu_release_addr[NR_CPUS];

static int __init smp_spin_table_init_cpu(struct device_node *dn, int cpu)
=======
#include <asm/cpu_ops.h>
#include <asm/cputype.h>
#include <asm/smp_plat.h>

static phys_addr_t cpu_release_addr[NR_CPUS];

/*
 * Write secondary_holding_pen_release in a way that is guaranteed to be
 * visible to all observers, irrespective of whether they're taking part
 * in coherency or not.  This is necessary for the hotplug code to work
 * reliably.
 */
static void write_pen_release(u64 val)
{
	void *start = (void *)&secondary_holding_pen_release;
	unsigned long size = sizeof(secondary_holding_pen_release);

	secondary_holding_pen_release = val;
	__flush_dcache_area(start, size);
}


static int smp_spin_table_cpu_init(struct device_node *dn, unsigned int cpu)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	/*
	 * Determine the address from which the CPU is polling.
	 */
	if (of_property_read_u64(dn, "cpu-release-addr",
				 &cpu_release_addr[cpu])) {
		pr_err("CPU %d: missing or invalid cpu-release-addr property\n",
		       cpu);

		return -1;
	}

	return 0;
}

<<<<<<< HEAD
static int __init smp_spin_table_prepare_cpu(int cpu)
=======
static int smp_spin_table_cpu_prepare(unsigned int cpu)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
{
	void **release_addr;

	if (!cpu_release_addr[cpu])
		return -ENODEV;

	release_addr = __va(cpu_release_addr[cpu]);
<<<<<<< HEAD
	release_addr[0] = (void *)__pa(secondary_holding_pen);
=======

	/*
	 * We write the release address as LE regardless of the native
	 * endianess of the kernel. Therefore, any boot-loaders that
	 * read this address need to convert this address to the
	 * boot-loader's endianess before jumping. This is mandated by
	 * the boot protocol.
	 */
	release_addr[0] = (void *) cpu_to_le64(__pa(secondary_holding_pen));

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	__flush_dcache_area(release_addr, sizeof(release_addr[0]));

	/*
	 * Send an event to wake up the secondary CPU.
	 */
	sev();

	return 0;
}

<<<<<<< HEAD
const struct smp_enable_ops smp_spin_table_ops __initconst = {
	.name		= "spin-table",
	.init_cpu 	= smp_spin_table_init_cpu,
	.prepare_cpu	= smp_spin_table_prepare_cpu,
};
=======
static int smp_spin_table_cpu_boot(unsigned int cpu)
{
	/*
	 * Update the pen release flag.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send an event, causing the secondaries to read pen_release.
	 */
	sev();

	return 0;
}

static const struct cpu_operations smp_spin_table_ops = {
	.name		= "spin-table",
	.cpu_init	= smp_spin_table_cpu_init,
	.cpu_prepare	= smp_spin_table_cpu_prepare,
	.cpu_boot	= smp_spin_table_cpu_boot,
};
CPU_METHOD_OF_DECLARE(spin_table, &smp_spin_table_ops);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
