/*
 * SMP initialisation and IPI support
 * Based on arch/arm/kernel/smp.c
 *
 * Copyright (C) 2012 ARM Ltd.
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

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/cache.h>
#include <linux/profile.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/percpu.h>
#include <linux/clockchips.h>
#include <linux/completion.h>
#include <linux/of.h>
<<<<<<< HEAD
=======
#include <linux/irq_work.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cputype.h>
<<<<<<< HEAD
=======
#include <asm/cpu_ops.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/processor.h>
#include <asm/smp_plat.h>
#include <asm/sections.h>
#include <asm/tlbflush.h>
#include <asm/ptrace.h>
<<<<<<< HEAD
=======
#include <asm/edac.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

/*
 * as from 2.5, kernels no longer have an init_tasks structure
 * so we need some other way of telling a new secondary core
 * where to place its SVC stack
 */
struct secondary_data secondary_data;
volatile unsigned long secondary_holding_pen_release = INVALID_HWID;

enum ipi_msg_type {
	IPI_RESCHEDULE,
	IPI_CALL_FUNC,
	IPI_CALL_FUNC_SINGLE,
	IPI_CPU_STOP,
<<<<<<< HEAD
};

static DEFINE_RAW_SPINLOCK(boot_lock);

/*
 * Write secondary_holding_pen_release in a way that is guaranteed to be
 * visible to all observers, irrespective of whether they're taking part
 * in coherency or not.  This is necessary for the hotplug code to work
 * reliably.
 */
static void __cpuinit write_pen_release(u64 val)
{
	void *start = (void *)&secondary_holding_pen_release;
	unsigned long size = sizeof(secondary_holding_pen_release);

	secondary_holding_pen_release = val;
	__flush_dcache_area(start, size);
}

=======
	IPI_TIMER,
	IPI_IRQ_WORK,
	IPI_WAKEUP,
	IPI_CPU_BACKTRACE,
};

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/*
 * Boot a secondary CPU, and assign it the specified idle task.
 * This also gives us the initial stack to use for this CPU.
 */
static int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
<<<<<<< HEAD
	unsigned long timeout;

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	raw_spin_lock(&boot_lock);

	/*
	 * Update the pen release flag.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send an event, causing the secondaries to read pen_release.
	 */
	sev();

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		if (secondary_holding_pen_release == INVALID_HWID)
			break;
		udelay(10);
	}

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	raw_spin_unlock(&boot_lock);

	return secondary_holding_pen_release != INVALID_HWID ? -ENOSYS : 0;
=======
	if (cpu_ops[cpu]->cpu_boot)
		return cpu_ops[cpu]->cpu_boot(cpu);

	return -EOPNOTSUPP;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static DECLARE_COMPLETION(cpu_running);

int __cpuinit __cpu_up(unsigned int cpu, struct task_struct *idle)
{
	int ret;

	/*
	 * We need to tell the secondary core where to find its stack and the
	 * page tables.
	 */
	secondary_data.stack = task_stack_page(idle) + THREAD_START_SP;
	__flush_dcache_area(&secondary_data, sizeof(secondary_data));

	/*
	 * Now bring the CPU into our world.
	 */
	ret = boot_secondary(cpu, idle);
	if (ret == 0) {
		/*
		 * CPU was successfully started, wait for it to come online or
		 * time out.
		 */
		wait_for_completion_timeout(&cpu_running,
					    msecs_to_jiffies(1000));

		if (!cpu_online(cpu)) {
			pr_crit("CPU%u: failed to come online\n", cpu);
			ret = -EIO;
		}
	} else {
		pr_err("CPU%u: failed to boot: %d\n", cpu, ret);
	}

	secondary_data.stack = NULL;

	return ret;
}

<<<<<<< HEAD
=======
static void smp_store_cpu_info(unsigned int cpuid)
{
	store_cpu_topology(cpuid);
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/*
 * This is the secondary CPU boot entry.  We're using this CPUs
 * idle thread stack, but a set of temporary page tables.
 */
asmlinkage void __cpuinit secondary_start_kernel(void)
{
	struct mm_struct *mm = &init_mm;
	unsigned int cpu = smp_processor_id();

<<<<<<< HEAD
	printk("CPU%u: Booted secondary processor\n", cpu);

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * All kernel threads share the same mm context; grab a
	 * reference and switch to it.
	 */
	atomic_inc(&mm->mm_count);
	current->active_mm = mm;
	cpumask_set_cpu(cpu, mm_cpumask(mm));

<<<<<<< HEAD
=======
	set_my_cpu_offset(per_cpu_offset(smp_processor_id()));
	printk("CPU%u: Booted secondary processor\n", cpu);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * TTBR0 is only used for the identity mapping at this stage. Make it
	 * point to zero page to avoid speculatively fetching new entries.
	 */
	cpu_set_reserved_ttbr0();
	flush_tlb_all();

	preempt_disable();
	trace_hardirqs_off();

<<<<<<< HEAD
	/*
	 * Let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(INVALID_HWID);

	/*
	 * Synchronise with the boot thread.
	 */
	raw_spin_lock(&boot_lock);
	raw_spin_unlock(&boot_lock);

	/*
	 * Enable local interrupts.
	 */
	notify_cpu_starting(cpu);
	local_irq_enable();
	local_fiq_enable();
=======
	if (cpu_ops[cpu]->cpu_postboot)
		cpu_ops[cpu]->cpu_postboot();

	/*
	 * Enable GIC and timers.
	 */
	smp_store_cpu_info(cpu);

	notify_cpu_starting(cpu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/*
	 * OK, now it's safe to let the boot CPU continue.  Wait for
	 * the CPU migration code to notice that the CPU is online
	 * before we continue.
	 */
	set_cpu_online(cpu, true);
	complete(&cpu_running);

<<<<<<< HEAD
=======
	local_dbg_enable();
	local_irq_enable();
	local_async_enable();

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * OK, it's off to the idle thread for us
	 */
	cpu_startup_entry(CPUHP_ONLINE);
}

<<<<<<< HEAD
void __init smp_cpus_done(unsigned int max_cpus)
{
	unsigned long bogosum = loops_per_jiffy * num_online_cpus();

	pr_info("SMP: Total of %d processors activated (%lu.%02lu BogoMIPS).\n",
		num_online_cpus(), bogosum / (500000/HZ),
		(bogosum / (5000/HZ)) % 100);
}

void __init smp_prepare_boot_cpu(void)
{
}

static void (*smp_cross_call)(const struct cpumask *, unsigned int);

static const struct smp_enable_ops *enable_ops[] __initconst = {
	&smp_spin_table_ops,
	&smp_psci_ops,
	NULL,
};

static const struct smp_enable_ops *smp_enable_ops[NR_CPUS];

static const struct smp_enable_ops * __init smp_get_enable_ops(const char *name)
{
	const struct smp_enable_ops **ops = enable_ops;

	while (*ops) {
		if (!strcmp(name, (*ops)->name))
			return *ops;

		ops++;
	}

	return NULL;
}

/*
=======
#ifdef CONFIG_HOTPLUG_CPU
static int op_cpu_disable(unsigned int cpu)
{
	/*
	 * If we don't have a cpu_die method, abort before we reach the point
	 * of no return. CPU0 may not have an cpu_ops, so test for it.
	 */
	if (!cpu_ops[cpu] || !cpu_ops[cpu]->cpu_die)
		return -EOPNOTSUPP;

	/*
	 * We may need to abort a hot unplug for some other mechanism-specific
	 * reason.
	 */
	if (cpu_ops[cpu]->cpu_disable)
		return cpu_ops[cpu]->cpu_disable(cpu);

	return 0;
}

/*
 * __cpu_disable runs on the processor to be shutdown.
 */
int __cpu_disable(void)
{
	unsigned int cpu = smp_processor_id();
	int ret;

	ret = op_cpu_disable(cpu);
	if (ret)
		return ret;

	/*
	 * Take this CPU offline.  Once we clear this, we can't return,
	 * and we must not schedule until we're ready to give up the cpu.
	 */
	set_cpu_online(cpu, false);

	/*
	 * OK - migrate IRQs away from this CPU
	 */
	migrate_irqs();

	/*
	 * Remove this CPU from the vm mask set of all processes.
	 */
	clear_tasks_mm_cpumask(cpu);

	return 0;
}

static int op_cpu_kill(unsigned int cpu)
{
	/*
	 * If we have no means of synchronising with the dying CPU, then assume
	 * that it is really dead. We can only wait for an arbitrary length of
	 * time and hope that it's dead, so let's skip the wait and just hope.
	 */
	if (!cpu_ops[cpu]->cpu_kill)
		return 1;

	return cpu_ops[cpu]->cpu_kill(cpu);
}

static DECLARE_COMPLETION(cpu_died);

/*
 * called on the thread which is asking for a CPU to be shutdown -
 * waits until shutdown has completed, or it is timed out.
 */
void __cpu_die(unsigned int cpu)
{
	if (!wait_for_completion_timeout(&cpu_died, msecs_to_jiffies(5000))) {
		pr_crit("CPU%u: cpu didn't die\n", cpu);
		return;
	}
	pr_notice("CPU%u: shutdown\n", cpu);

	/*
	 * Now that the dying CPU is beyond the point of no return w.r.t.
	 * in-kernel synchronisation, try to get the firwmare to help us to
	 * verify that it has really left the kernel before we consider
	 * clobbering anything it might still be using.
	 */
	if (!op_cpu_kill(cpu))
		pr_warn("CPU%d may not have shut down cleanly\n", cpu);
}

/*
 * Called from the idle thread for the CPU which has been shutdown.
 *
 * Note that we disable IRQs here, but do not re-enable them
 * before returning to the caller. This is also the behaviour
 * of the other hotplug-cpu capable cores, so presumably coming
 * out of idle fixes this.
 */
void __ref cpu_die(void)
{
	unsigned int cpu = smp_processor_id();

	idle_task_exit();

	local_irq_disable();

	/* Tell __cpu_die() that this CPU is now safe to dispose of */
	complete(&cpu_died);

	/*
	 * Actually shutdown the CPU. This must never fail. The specific hotplug
	 * mechanism must perform all required cache maintenance to ensure that
	 * no dirty lines are lost in the process of shutting down the CPU.
	 */
	cpu_ops[cpu]->cpu_die(cpu);

	/*
	 * Do not return to the idle loop - jump back to the secondary
	 * cpu initialisation.  There's some initialisation which needs
	 * to be repeated to undo the effects of taking the CPU offline.
	 */

	asm volatile("mov       sp, %0\n"
		     "mov       x29, #0\n"
		     "b         secondary_start_kernel"
		     : : "r" (task_stack_page(current) + THREAD_START_SP));
}
#endif

void __init smp_cpus_done(unsigned int max_cpus)
{
	pr_info("SMP: Total of %d processors activated.\n", num_online_cpus());
}

void __init smp_prepare_boot_cpu(void)
{
	set_my_cpu_offset(per_cpu_offset(smp_processor_id()));
}

static void (*smp_cross_call)(const struct cpumask *, unsigned int);
DEFINE_PER_CPU(bool, pending_ipi);

void smp_cross_call_common(const struct cpumask *cpumask, unsigned int func)
{
	unsigned int cpu;

	for_each_cpu(cpu, cpumask)
		per_cpu(pending_ipi, cpu) = true;

	smp_cross_call(cpumask, func);
}


/*
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
 * Enumerate the possible CPU set from the device tree and build the
 * cpu logical map array containing MPIDR values related to logical
 * cpus. Assumes that cpu_logical_map(0) has already been initialized.
 */
void __init smp_init_cpus(void)
{
<<<<<<< HEAD
	const char *enable_method;
	struct device_node *dn = NULL;
	int i, cpu = 1;
=======
	struct device_node *dn = NULL;
	unsigned int i, cpu = 1;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	bool bootcpu_valid = false;

	while ((dn = of_find_node_by_type(dn, "cpu"))) {
		const u32 *cell;
		u64 hwid;

		/*
		 * A cpu node with missing "reg" property is
		 * considered invalid to build a cpu_logical_map
		 * entry.
		 */
		cell = of_get_property(dn, "reg", NULL);
		if (!cell) {
			pr_err("%s: missing reg property\n", dn->full_name);
			goto next;
		}
		hwid = of_read_number(cell, of_n_addr_cells(dn));

		/*
		 * Non affinity bits must be set to 0 in the DT
		 */
		if (hwid & ~MPIDR_HWID_BITMASK) {
			pr_err("%s: invalid reg property\n", dn->full_name);
			goto next;
		}

		/*
		 * Duplicate MPIDRs are a recipe for disaster. Scan
		 * all initialized entries and check for
		 * duplicates. If any is found just ignore the cpu.
		 * cpu_logical_map was initialized to INVALID_HWID to
		 * avoid matching valid MPIDR values.
		 */
		for (i = 1; (i < cpu) && (i < NR_CPUS); i++) {
			if (cpu_logical_map(i) == hwid) {
				pr_err("%s: duplicate cpu reg properties in the DT\n",
					dn->full_name);
				goto next;
			}
		}

		/*
		 * The numbering scheme requires that the boot CPU
		 * must be assigned logical id 0. Record it so that
		 * the logical map built from DT is validated and can
		 * be used.
		 */
		if (hwid == cpu_logical_map(0)) {
			if (bootcpu_valid) {
				pr_err("%s: duplicate boot cpu reg property in DT\n",
					dn->full_name);
				goto next;
			}

			bootcpu_valid = true;

			/*
			 * cpu_logical_map has already been
			 * initialized and the boot cpu doesn't need
			 * the enable-method so continue without
			 * incrementing cpu.
			 */
			continue;
		}

		if (cpu >= NR_CPUS)
			goto next;

<<<<<<< HEAD
		/*
		 * We currently support only the "spin-table" enable-method.
		 */
		enable_method = of_get_property(dn, "enable-method", NULL);
		if (!enable_method) {
			pr_err("%s: missing enable-method property\n",
				dn->full_name);
			goto next;
		}

		smp_enable_ops[cpu] = smp_get_enable_ops(enable_method);

		if (!smp_enable_ops[cpu]) {
			pr_err("%s: invalid enable-method property: %s\n",
			       dn->full_name, enable_method);
			goto next;
		}

		if (smp_enable_ops[cpu]->init_cpu(dn, cpu))
=======
		if (cpu_read_ops(dn, cpu) != 0)
			goto next;

		if (cpu_ops[cpu]->cpu_init(dn, cpu))
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			goto next;

		pr_debug("cpu logical map 0x%llx\n", hwid);
		cpu_logical_map(cpu) = hwid;
next:
		cpu++;
	}

	/* sanity check */
	if (cpu > NR_CPUS)
		pr_warning("no. of cores (%d) greater than configured maximum of %d - clipping\n",
			   cpu, NR_CPUS);

	if (!bootcpu_valid) {
		pr_err("DT missing boot CPU MPIDR, not enabling secondaries\n");
		return;
	}

	/*
	 * All the cpus that made it to the cpu_logical_map have been
	 * validated so set them as possible cpus.
	 */
	for (i = 0; i < NR_CPUS; i++)
		if (cpu_logical_map(i) != INVALID_HWID)
			set_cpu_possible(i, true);
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
<<<<<<< HEAD
	int cpu, err;
	unsigned int ncores = num_possible_cpus();
=======
	int err;
	unsigned int cpu, ncores = num_possible_cpus();

	init_cpu_topology();

	smp_store_cpu_info(smp_processor_id());
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/*
	 * are we trying to boot more cores than exist?
	 */
	if (max_cpus > ncores)
		max_cpus = ncores;

	/* Don't bother if we're effectively UP */
	if (max_cpus <= 1)
		return;

	/*
	 * Initialise the present map (which describes the set of CPUs
	 * actually populated at the present time) and release the
	 * secondaries from the bootloader.
	 *
	 * Make sure we online at most (max_cpus - 1) additional CPUs.
	 */
	max_cpus--;
	for_each_possible_cpu(cpu) {
		if (max_cpus == 0)
			break;

		if (cpu == smp_processor_id())
			continue;

<<<<<<< HEAD
		if (!smp_enable_ops[cpu])
			continue;

		err = smp_enable_ops[cpu]->prepare_cpu(cpu);
=======
		if (!cpu_ops[cpu])
			continue;

		err = cpu_ops[cpu]->cpu_prepare(cpu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (err)
			continue;

		set_cpu_present(cpu, true);
		max_cpus--;
	}
}

<<<<<<< HEAD

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
void __init set_smp_cross_call(void (*fn)(const struct cpumask *, unsigned int))
{
	smp_cross_call = fn;
}

void arch_send_call_function_ipi_mask(const struct cpumask *mask)
{
<<<<<<< HEAD
	smp_cross_call(mask, IPI_CALL_FUNC);
=======
	smp_cross_call_common(mask, IPI_CALL_FUNC);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

void arch_send_call_function_single_ipi(int cpu)
{
<<<<<<< HEAD
	smp_cross_call(cpumask_of(cpu), IPI_CALL_FUNC_SINGLE);
}
=======
	smp_cross_call_common(cpumask_of(cpu), IPI_CALL_FUNC_SINGLE);
}

void arch_send_wakeup_ipi_mask(const struct cpumask *mask)
{
	smp_cross_call_common(mask, IPI_WAKEUP);
}

#ifdef CONFIG_IRQ_WORK
void arch_irq_work_raise(void)
{
	if (smp_cross_call)
		smp_cross_call(cpumask_of(smp_processor_id()), IPI_IRQ_WORK);
}
#endif
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

static const char *ipi_types[NR_IPI] = {
#define S(x,s)	[x - IPI_RESCHEDULE] = s
	S(IPI_RESCHEDULE, "Rescheduling interrupts"),
	S(IPI_CALL_FUNC, "Function call interrupts"),
	S(IPI_CALL_FUNC_SINGLE, "Single function call interrupts"),
	S(IPI_CPU_STOP, "CPU stop interrupts"),
<<<<<<< HEAD
=======
	S(IPI_TIMER, "Timer broadcast interrupts"),
	S(IPI_IRQ_WORK, "IRQ work interrupts"),
	S(IPI_WAKEUP, "CPU wakeup interrupts"),
	S(IPI_CPU_BACKTRACE, "CPU backtrace"),
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
};

void show_ipi_list(struct seq_file *p, int prec)
{
	unsigned int cpu, i;

	for (i = 0; i < NR_IPI; i++) {
		seq_printf(p, "%*s%u:%s", prec - 1, "IPI", i + IPI_RESCHEDULE,
			   prec >= 4 ? " " : "");
<<<<<<< HEAD
		for_each_present_cpu(cpu)
=======
		for_each_online_cpu(cpu)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			seq_printf(p, "%10u ",
				   __get_irq_stat(cpu, ipi_irqs[i]));
		seq_printf(p, "      %s\n", ipi_types[i]);
	}
}

u64 smp_irq_stat_cpu(unsigned int cpu)
{
	u64 sum = 0;
	int i;

	for (i = 0; i < NR_IPI; i++)
		sum += __get_irq_stat(cpu, ipi_irqs[i]);

	return sum;
}

static DEFINE_RAW_SPINLOCK(stop_lock);

<<<<<<< HEAD
/*
 * ipi_cpu_stop - handle IPI from smp_send_stop()
 */
static void ipi_cpu_stop(unsigned int cpu)
{
	if (system_state == SYSTEM_BOOTING ||
	    system_state == SYSTEM_RUNNING) {
		raw_spin_lock(&stop_lock);
		pr_crit("CPU%u: stopping\n", cpu);
		dump_stack();
=======
DEFINE_PER_CPU(struct pt_regs, regs_before_stop);

/*
 * ipi_cpu_stop - handle IPI from smp_send_stop()
 */
static void ipi_cpu_stop(unsigned int cpu, struct pt_regs *regs)
{
	if (system_state == SYSTEM_BOOTING ||
	    system_state == SYSTEM_RUNNING) {
		per_cpu(regs_before_stop, cpu) = *regs;
		raw_spin_lock(&stop_lock);
		pr_crit("CPU%u: stopping\n", cpu);
		show_regs(regs);
		dump_stack();
		arm64_check_cache_ecc(NULL);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		raw_spin_unlock(&stop_lock);
	}

	set_cpu_online(cpu, false);

<<<<<<< HEAD
	local_fiq_disable();
=======
	flush_cache_all();
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	local_irq_disable();

	while (1)
		cpu_relax();
}

<<<<<<< HEAD
=======
static cpumask_t backtrace_mask;
static DEFINE_RAW_SPINLOCK(backtrace_lock);

/* "in progress" flag of arch_trigger_all_cpu_backtrace */
static unsigned long backtrace_flag;

static void smp_send_all_cpu_backtrace(void)
{
	unsigned int this_cpu = smp_processor_id();
	int i;

	if (test_and_set_bit(0, &backtrace_flag))
		/*
		 * If there is already a trigger_all_cpu_backtrace() in progress
		 * (backtrace_flag == 1), don't output double cpu dump infos.
		 */
		return;

	cpumask_copy(&backtrace_mask, cpu_online_mask);
	cpu_clear(this_cpu, backtrace_mask);

	pr_info("Backtrace for cpu %d (current):\n", this_cpu);
	dump_stack();

	pr_info("\nsending IPI to all other CPUs:\n");
	if (!cpus_empty(backtrace_mask))
		smp_cross_call_common(&backtrace_mask, IPI_CPU_BACKTRACE);

	/* Wait for up to 10 seconds for all other CPUs to do the backtrace */
	for (i = 0; i < 10 * 1000; i++) {
		if (cpumask_empty(&backtrace_mask))
			break;
		mdelay(1);
	}

	clear_bit(0, &backtrace_flag);
	smp_mb__after_atomic();
}

/*
 * ipi_cpu_backtrace - handle IPI from smp_send_all_cpu_backtrace()
 */
static void ipi_cpu_backtrace(unsigned int cpu, struct pt_regs *regs)
{
	if (cpu_isset(cpu, backtrace_mask)) {
		raw_spin_lock(&backtrace_lock);
		pr_warn("IPI backtrace for cpu %d\n", cpu);
		show_regs(regs);
		raw_spin_unlock(&backtrace_lock);
		cpu_clear(cpu, backtrace_mask);
	}
}

#ifdef CONFIG_SMP
void arch_trigger_all_cpu_backtrace(void)
{
	smp_send_all_cpu_backtrace();
}
#else
void arch_trigger_all_cpu_backtrace(void)
{
	dump_stack();
}
#endif


>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/*
 * Main handler for inter-processor interrupts
 */
void handle_IPI(int ipinr, struct pt_regs *regs)
{
	unsigned int cpu = smp_processor_id();
	struct pt_regs *old_regs = set_irq_regs(regs);

	if (ipinr >= IPI_RESCHEDULE && ipinr < IPI_RESCHEDULE + NR_IPI)
		__inc_irq_stat(cpu, ipi_irqs[ipinr - IPI_RESCHEDULE]);

	switch (ipinr) {
	case IPI_RESCHEDULE:
		scheduler_ipi();
		break;

	case IPI_CALL_FUNC:
		irq_enter();
		generic_smp_call_function_interrupt();
		irq_exit();
		break;

	case IPI_CALL_FUNC_SINGLE:
		irq_enter();
		generic_smp_call_function_single_interrupt();
		irq_exit();
		break;

	case IPI_CPU_STOP:
		irq_enter();
<<<<<<< HEAD
		ipi_cpu_stop(cpu);
		irq_exit();
		break;
=======
		ipi_cpu_stop(cpu, regs);
		irq_exit();
		break;

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
	case IPI_TIMER:
		irq_enter();
		tick_receive_broadcast();
		irq_exit();
		break;
#endif
	case IPI_WAKEUP:
		break;

	case IPI_CPU_BACKTRACE:
		ipi_cpu_backtrace(cpu, regs);
		break;

#ifdef CONFIG_IRQ_WORK
	case IPI_IRQ_WORK:
		irq_enter();
		irq_work_run();
		irq_exit();
		break;
#endif
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	default:
		pr_crit("CPU%u: Unknown IPI message 0x%x\n", cpu, ipinr);
		break;
	}
<<<<<<< HEAD
=======
	per_cpu(pending_ipi, cpu) = false;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	set_irq_regs(old_regs);
}

void smp_send_reschedule(int cpu)
{
<<<<<<< HEAD
	smp_cross_call(cpumask_of(cpu), IPI_RESCHEDULE);
}
=======
	BUG_ON(cpu_is_offline(cpu));
	smp_cross_call_common(cpumask_of(cpu), IPI_RESCHEDULE);
}

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
void tick_broadcast(const struct cpumask *mask)
{
	smp_cross_call_common(mask, IPI_TIMER);
}
#endif
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

void smp_send_stop(void)
{
	unsigned long timeout;

	if (num_online_cpus() > 1) {
		cpumask_t mask;

		cpumask_copy(&mask, cpu_online_mask);
		cpu_clear(smp_processor_id(), mask);

<<<<<<< HEAD
		smp_cross_call(&mask, IPI_CPU_STOP);
=======
		smp_cross_call_common(&mask, IPI_CPU_STOP);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	/* Wait up to one second for other CPUs to stop */
	timeout = USEC_PER_SEC;
	while (num_online_cpus() > 1 && timeout--)
		udelay(1);

	if (num_online_cpus() > 1)
		pr_warning("SMP: failed to stop secondary CPUs\n");
}

/*
 * not supported here
 */
int setup_profiling_timer(unsigned int multiplier)
{
	return -EINVAL;
}
