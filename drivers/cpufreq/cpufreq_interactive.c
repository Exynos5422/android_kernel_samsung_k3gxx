/*
 * drivers/cpufreq/cpufreq_interactive.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Mike Chan (mike@android.com)
 *
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
<<<<<<< HEAD
#include <linux/ipa.h>
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/rwsem.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/tick.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/kernel_stat.h>
<<<<<<< HEAD
#include <linux/pm_qos.h>
#include <asm/cputime.h>
#ifdef CONFIG_ANDROID
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/android_aid.h>
#endif

#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
#include <mach/cpufreq.h>
#endif
#include "cpu_load_metric.h"
=======
#include <asm/cputime.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#define CREATE_TRACE_POINTS
#include <trace/events/cpufreq_interactive.h>

struct cpufreq_interactive_cpuinfo {
	struct timer_list cpu_timer;
	struct timer_list cpu_slack_timer;
	spinlock_t load_lock; /* protects the next 4 fields */
	u64 time_in_idle;
	u64 time_in_idle_timestamp;
	u64 cputime_speedadj;
	u64 cputime_speedadj_timestamp;
<<<<<<< HEAD
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;
	unsigned int target_freq;
	unsigned int floor_freq;
	u64 floor_validate_time;
	u64 hispeed_validate_time;
	struct rw_semaphore enable_sem;
	int governor_enabled;
=======
	u64 last_evaluated_jiffy;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;
	spinlock_t target_freq_lock; /*protects target freq */
	unsigned int target_freq;
	unsigned int floor_freq;
	unsigned int max_freq;
	u64 floor_validate_time;
	u64 local_fvtime; /* per-cpu floor_validate_time */
	u64 hispeed_validate_time; /* cluster hispeed_validate_time */
	u64 local_hvtime; /* per-cpu hispeed_validate_time */
	u64 max_freq_idle_start_time;
	struct rw_semaphore enable_sem;
	int governor_enabled;
	struct cpufreq_interactive_tunables *cached_tunables;
	int first_cpu;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
};

static DEFINE_PER_CPU(struct cpufreq_interactive_cpuinfo, cpuinfo);

<<<<<<< HEAD
=======
/* realtime thread handles frequency scaling */
static struct task_struct *speedchange_task;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static cpumask_t speedchange_cpumask;
static spinlock_t speedchange_cpumask_lock;
static struct mutex gov_lock;

<<<<<<< HEAD
=======
static int set_window_count;
static int migration_register_count;
static struct mutex sched_lock;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/* Target load.  Lower values result in higher CPU speeds. */
#define DEFAULT_TARGET_LOAD 90
static unsigned int default_target_loads[] = {DEFAULT_TARGET_LOAD};

#define DEFAULT_TIMER_RATE (20 * USEC_PER_MSEC)
#define DEFAULT_ABOVE_HISPEED_DELAY DEFAULT_TIMER_RATE
static unsigned int default_above_hispeed_delay[] = {
	DEFAULT_ABOVE_HISPEED_DELAY };

<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
extern struct cpumask hmp_fast_cpu_mask;
struct cpufreq_loadinfo {
	unsigned int load;
	unsigned int freq;
	u64 timestamp;
};
static bool hmp_boost;

#define MULTI_MODE  2
#define SINGLE_MODE 1
#define NO_MODE     0
#define DEFAULT_MULTI_ENTER_TIME (4 * DEFAULT_TIMER_RATE)
#define DEFAULT_MULTI_EXIT_TIME (16 * DEFAULT_TIMER_RATE)
#define DEFAULT_SINGLE_ENTER_TIME (8 * DEFAULT_TIMER_RATE)
#define DEFAULT_SINGLE_EXIT_TIME (4 * DEFAULT_TIMER_RATE)

static DEFINE_PER_CPU(struct cpufreq_loadinfo, loadinfo);
static DEFINE_PER_CPU(unsigned int, cpu_util);
#endif /* CONFIG_MODE_AUTO_CHANGE */

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
struct cpufreq_interactive_tunables {
	int usage_count;
	/* Hi speed to bump to from lo speed when load burst (default max) */
	unsigned int hispeed_freq;
	/* Go to hi speed when CPU load at or above this value. */
#define DEFAULT_GO_HISPEED_LOAD 99
	unsigned long go_hispeed_load;
	/* Target load. Lower values result in higher CPU speeds. */
	spinlock_t target_loads_lock;
	unsigned int *target_loads;
	int ntarget_loads;
	/*
	 * The minimum amount of time to spend at a frequency before we can ramp
	 * down.
	 */
#define DEFAULT_MIN_SAMPLE_TIME (80 * USEC_PER_MSEC)
	unsigned long min_sample_time;
	/*
	 * The sample rate of the timer used to increase frequency
	 */
	unsigned long timer_rate;
	/*
	 * Wait this long before raising speed above hispeed, by default a
	 * single timer interval.
	 */
	spinlock_t above_hispeed_delay_lock;
	unsigned int *above_hispeed_delay;
	int nabove_hispeed_delay;
	/* Non-zero means indefinite speed boost active */
	int boost_val;
	/* Duration of a boot pulse in usecs */
	int boostpulse_duration_val;
	/* End time of boost pulse in ktime converted to usecs */
	u64 boostpulse_endtime;
<<<<<<< HEAD
=======
	bool boosted;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * Max additional time to wait in idle, beyond timer_rate, at speeds
	 * above minimum before wakeup to reduce speed, or -1 if unnecessary.
	 */
#define DEFAULT_TIMER_SLACK (4 * DEFAULT_TIMER_RATE)
	int timer_slack_val;
	bool io_is_busy;

<<<<<<< HEAD
#define TASK_NAME_LEN 15
	/* realtime thread handles frequency scaling */
	struct task_struct *speedchange_task;

	/* handle for get cpufreq_policy */
	unsigned int *policy;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spinlock_t mode_lock;
	unsigned int mode;
	unsigned int enforced_mode;
	u64 mode_check_timestamp;

	unsigned long multi_enter_time;
	unsigned long time_in_multi_enter;
	unsigned int multi_enter_load;

	unsigned long multi_exit_time;
	unsigned long time_in_multi_exit;
	unsigned int multi_exit_load;

	unsigned long single_enter_time;
	unsigned long time_in_single_enter;
	unsigned int single_enter_load;

	unsigned long single_exit_time;
	unsigned long time_in_single_exit;
	unsigned int single_exit_load;

	spinlock_t param_index_lock;
	unsigned int param_index;
	unsigned int cur_param_index;

#define MAX_PARAM_SET 4 /* ((MULTI_MODE | SINGLE_MODE | NO_MODE) + 1) */
	unsigned int hispeed_freq_set[MAX_PARAM_SET];
	unsigned long go_hispeed_load_set[MAX_PARAM_SET];
	unsigned int *target_loads_set[MAX_PARAM_SET];
	int ntarget_loads_set[MAX_PARAM_SET];
	unsigned long min_sample_time_set[MAX_PARAM_SET];
	unsigned long timer_rate_set[MAX_PARAM_SET];
	unsigned int *above_hispeed_delay_set[MAX_PARAM_SET];
	int nabove_hispeed_delay_set[MAX_PARAM_SET];
	unsigned int sampling_down_factor_set[MAX_PARAM_SET];

	unsigned int sampling_down_factor;
#endif
};

/* For cases where we have single governor instance for system */
struct cpufreq_interactive_tunables *common_tunables;
static struct cpufreq_interactive_tunables *tuned_parameters[NR_CPUS] = {NULL,};

static struct attribute_group *get_sysfs_attr(void);

static inline cputime64_t get_cpu_idle_time_jiffy(unsigned int cpu,
						  cputime64_t *wall)
{
	u64 idle_time;
	u64 cur_wall_time;
	u64 busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	busy_time  = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];

	idle_time = cur_wall_time - busy_time;
	if (wall)
		*wall = jiffies_to_usecs(cur_wall_time);

	return jiffies_to_usecs(idle_time);
}

static inline cputime64_t get_cpu_idle_time(
	unsigned int cpu,
	cputime64_t *wall,
	bool io_is_busy)
{
	u64 idle_time = get_cpu_idle_time_us(cpu, wall);

	if (idle_time == -1ULL)
		idle_time = get_cpu_idle_time_jiffy(cpu, wall);
	else if (!io_is_busy)
		idle_time += get_cpu_iowait_time_us(cpu, wall);

	return idle_time;
}

static void cpufreq_interactive_timer_resched(
	struct cpufreq_interactive_cpuinfo *pcpu)
{
	struct cpufreq_interactive_tunables *tunables =
		pcpu->policy->governor_data;
	unsigned long expires;
	unsigned long flags;

	if (!tunables->speedchange_task)
		return;

=======
	/* scheduler input related flags */
	bool use_sched_load;
	bool use_migration_notif;

	/*
	 * Whether to align timer windows across all CPUs. When
	 * use_sched_load is true, this flag is ignored and windows
	 * will always be aligned.
	 */
	bool align_windows;

	/*
	 * Stay at max freq for at least max_freq_hysteresis before dropping
	 * frequency.
	 */
	unsigned int max_freq_hysteresis;
};

/* For cases where we have single governor instance for system */
static struct cpufreq_interactive_tunables *common_tunables;

static struct attribute_group *get_sysfs_attr(void);

/* Round to starting jiffy of next evaluation window */
static u64 round_to_nw_start(u64 jif,
			     struct cpufreq_interactive_tunables *tunables)
{
	unsigned long step = usecs_to_jiffies(tunables->timer_rate);
	u64 ret;

	if (tunables->use_sched_load || tunables->align_windows) {
		do_div(jif, step);
		ret = (jif + 1) * step;
	} else {
		ret = jiffies + usecs_to_jiffies(tunables->timer_rate);
	}

	return ret;
}

static inline int set_window_helper(
			struct cpufreq_interactive_tunables *tunables)
{
	return sched_set_window(round_to_nw_start(get_jiffies_64(), tunables),
			 usecs_to_jiffies(tunables->timer_rate));
}

static void cpufreq_interactive_timer_resched(unsigned long cpu)
{
	struct cpufreq_interactive_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
	struct cpufreq_interactive_tunables *tunables =
		pcpu->policy->governor_data;
	u64 expires;
	unsigned long flags;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	spin_lock_irqsave(&pcpu->load_lock, flags);
	pcpu->time_in_idle =
		get_cpu_idle_time(smp_processor_id(),
				  &pcpu->time_in_idle_timestamp,
				  tunables->io_is_busy);
	pcpu->cputime_speedadj = 0;
	pcpu->cputime_speedadj_timestamp = pcpu->time_in_idle_timestamp;
<<<<<<< HEAD
	expires = jiffies + usecs_to_jiffies(tunables->timer_rate);
	mod_timer_pinned(&pcpu->cpu_timer, expires);
=======
	expires = round_to_nw_start(pcpu->last_evaluated_jiffy, tunables);
	del_timer(&pcpu->cpu_timer);
	pcpu->cpu_timer.expires = expires;
	add_timer_on(&pcpu->cpu_timer, cpu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (tunables->timer_slack_val >= 0 &&
	    pcpu->target_freq > pcpu->policy->min) {
		expires += usecs_to_jiffies(tunables->timer_slack_val);
<<<<<<< HEAD
		mod_timer_pinned(&pcpu->cpu_slack_timer, expires);
=======
		del_timer(&pcpu->cpu_slack_timer);
		pcpu->cpu_slack_timer.expires = expires;
		add_timer_on(&pcpu->cpu_slack_timer, cpu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	spin_unlock_irqrestore(&pcpu->load_lock, flags);
}

/* The caller shall take enable_sem write semaphore to avoid any timer race.
 * The cpu_timer and cpu_slack_timer must be deactivated when calling this
 * function.
 */
static void cpufreq_interactive_timer_start(
	struct cpufreq_interactive_tunables *tunables, int cpu)
{
	struct cpufreq_interactive_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
<<<<<<< HEAD
	unsigned long expires = jiffies +
		usecs_to_jiffies(tunables->timer_rate);
	unsigned long flags;

	if (!tunables->speedchange_task)
		return;

=======
	u64 expires = round_to_nw_start(pcpu->last_evaluated_jiffy, tunables);
	unsigned long flags;

	spin_lock_irqsave(&pcpu->load_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	pcpu->cpu_timer.expires = expires;
	add_timer_on(&pcpu->cpu_timer, cpu);
	if (tunables->timer_slack_val >= 0 &&
	    pcpu->target_freq > pcpu->policy->min) {
		expires += usecs_to_jiffies(tunables->timer_slack_val);
		pcpu->cpu_slack_timer.expires = expires;
		add_timer_on(&pcpu->cpu_slack_timer, cpu);
	}

<<<<<<< HEAD
	spin_lock_irqsave(&pcpu->load_lock, flags);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	pcpu->time_in_idle =
		get_cpu_idle_time(cpu, &pcpu->time_in_idle_timestamp,
				  tunables->io_is_busy);
	pcpu->cputime_speedadj = 0;
	pcpu->cputime_speedadj_timestamp = pcpu->time_in_idle_timestamp;
	spin_unlock_irqrestore(&pcpu->load_lock, flags);
}

static unsigned int freq_to_above_hispeed_delay(
	struct cpufreq_interactive_tunables *tunables,
	unsigned int freq)
{
	int i;
	unsigned int ret;
	unsigned long flags;

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	for (i = 0; i < tunables->nabove_hispeed_delay - 1 &&
			freq >= tunables->above_hispeed_delay[i+1]; i += 2)
		;

	ret = tunables->above_hispeed_delay[i];
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);
	return ret;
}

static unsigned int freq_to_targetload(
	struct cpufreq_interactive_tunables *tunables, unsigned int freq)
{
	int i;
	unsigned int ret;
	unsigned long flags;

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads - 1 &&
		    freq >= tunables->target_loads[i+1]; i += 2)
		;

	ret = tunables->target_loads[i];
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);
	return ret;
}

/*
 * If increasing frequencies never map to a lower target load then
 * choose_freq() will find the minimum frequency that does not exceed its
 * target load given the current load.
 */
static unsigned int choose_freq(struct cpufreq_interactive_cpuinfo *pcpu,
		unsigned int loadadjfreq)
{
	unsigned int freq = pcpu->policy->cur;
	unsigned int prevfreq, freqmin, freqmax;
	unsigned int tl;
	int index;

	freqmin = 0;
	freqmax = UINT_MAX;

	do {
		prevfreq = freq;
		tl = freq_to_targetload(pcpu->policy->governor_data, freq);

		/*
		 * Find the lowest frequency where the computed load is less
		 * than or equal to the target load.
		 */

		if (cpufreq_frequency_table_target(
			    pcpu->policy, pcpu->freq_table, loadadjfreq / tl,
			    CPUFREQ_RELATION_L, &index))
			break;
		freq = pcpu->freq_table[index].frequency;

		if (freq > prevfreq) {
			/* The previous frequency is too low. */
			freqmin = prevfreq;

			if (freq >= freqmax) {
				/*
				 * Find the highest frequency that is less
				 * than freqmax.
				 */
				if (cpufreq_frequency_table_target(
					    pcpu->policy, pcpu->freq_table,
					    freqmax - 1, CPUFREQ_RELATION_H,
					    &index))
					break;
				freq = pcpu->freq_table[index].frequency;

				if (freq == freqmin) {
					/*
					 * The first frequency below freqmax
					 * has already been found to be too
					 * low.  freqmax is the lowest speed
					 * we found that is fast enough.
					 */
					freq = freqmax;
					break;
				}
			}
		} else if (freq < prevfreq) {
			/* The previous frequency is high enough. */
			freqmax = prevfreq;

			if (freq <= freqmin) {
				/*
				 * Find the lowest frequency that is higher
				 * than freqmin.
				 */
				if (cpufreq_frequency_table_target(
					    pcpu->policy, pcpu->freq_table,
					    freqmin + 1, CPUFREQ_RELATION_L,
					    &index))
					break;
				freq = pcpu->freq_table[index].frequency;

				/*
				 * If freqmax is the first frequency above
				 * freqmin then we have already found that
				 * this speed is fast enough.
				 */
				if (freq == freqmax)
					break;
			}
		}

		/* If same frequency chosen as previous then done. */
	} while (freq != prevfreq);

	return freq;
}

static u64 update_load(int cpu)
{
	struct cpufreq_interactive_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
	struct cpufreq_interactive_tunables *tunables =
		pcpu->policy->governor_data;
	u64 now;
	u64 now_idle;
	unsigned int delta_idle;
	unsigned int delta_time;
	u64 active_time;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned int cur_load = 0;
	struct cpufreq_loadinfo *cur_loadinfo = &per_cpu(loadinfo, cpu);
#endif
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	now_idle = get_cpu_idle_time(cpu, &now, tunables->io_is_busy);
	delta_idle = (unsigned int)(now_idle - pcpu->time_in_idle);
	delta_time = (unsigned int)(now - pcpu->time_in_idle_timestamp);

	if (delta_time <= delta_idle)
		active_time = 0;
	else
		active_time = delta_time - delta_idle;

	pcpu->cputime_speedadj += active_time * pcpu->policy->cur;

<<<<<<< HEAD
	update_cpu_metric(cpu, now, delta_idle, delta_time, pcpu->policy);

	pcpu->time_in_idle = now_idle;
	pcpu->time_in_idle_timestamp = now;
#ifdef CONFIG_MODE_AUTO_CHANGE
	cur_load = (unsigned int)(active_time * 100) / delta_time;
	per_cpu(cpu_util, cpu) = cur_load;

	cur_loadinfo->load = (cur_load * pcpu->policy->cur) /
                    pcpu->policy->cpuinfo.max_freq;
	cur_loadinfo->freq = pcpu->policy->cur;
	cur_loadinfo->timestamp = now;
#endif
	return now;
}

#ifdef CONFIG_MODE_AUTO_CHANGE
static unsigned int check_mode(int cpu, unsigned int cur_mode, u64 now)
{
	int i;
	unsigned int ret=cur_mode, total_load=0, max_single_load=0;
	struct cpufreq_loadinfo *cur_loadinfo;
	struct cpufreq_interactive_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
	struct cpufreq_interactive_tunables *tunables =
		pcpu->policy->governor_data;

	if (now - tunables->mode_check_timestamp < tunables->timer_rate - USEC_PER_MSEC)
		return ret;

	if (now - tunables->mode_check_timestamp > tunables->timer_rate + USEC_PER_MSEC)
		tunables->mode_check_timestamp = now - tunables->timer_rate;

	if(cpumask_test_cpu(cpu, &hmp_fast_cpu_mask)) {
		for_each_cpu_mask(i, hmp_fast_cpu_mask) {
			cur_loadinfo = &per_cpu(loadinfo, i);
			if (now - cur_loadinfo->timestamp <= tunables->timer_rate + USEC_PER_MSEC) {
				total_load += cur_loadinfo->load;
				if (cur_loadinfo->load > max_single_load)
					max_single_load = cur_loadinfo->load;
			}
		}
	}
	else
		return ret;

	if (!(cur_mode & SINGLE_MODE)) {
		if (max_single_load >= tunables->single_enter_load)
			tunables->time_in_single_enter += now - tunables->mode_check_timestamp;
		else
			tunables->time_in_single_enter = 0;

		if (tunables->time_in_single_enter >= tunables->single_enter_time)
			ret |= SINGLE_MODE;
	}

	if (!(cur_mode & MULTI_MODE)) {
		if (total_load >= tunables->multi_enter_load)
			tunables->time_in_multi_enter += now - tunables->mode_check_timestamp;
		else
			tunables->time_in_multi_enter = 0;

		if (tunables->time_in_multi_enter >= tunables->multi_enter_time)
			ret |= MULTI_MODE;
	}

	if (cur_mode & SINGLE_MODE) {
		if (max_single_load < tunables->single_exit_load)
			tunables->time_in_single_exit += now - tunables->mode_check_timestamp;
		else
			tunables->time_in_single_exit = 0;

		if (tunables->time_in_single_exit >= tunables->single_exit_time)
			ret &= ~SINGLE_MODE;
	}

	if (cur_mode & MULTI_MODE) {
		if (total_load < tunables->multi_exit_load)
			tunables->time_in_multi_exit += now - tunables->mode_check_timestamp;
		else
			tunables->time_in_multi_exit = 0;

		if (tunables->time_in_multi_exit >= tunables->multi_exit_time)
			ret &= ~MULTI_MODE;
	}

	trace_cpufreq_interactive_mode(cpu, total_load,
		tunables->time_in_single_enter, tunables->time_in_multi_enter,
		tunables->time_in_single_exit, tunables->time_in_multi_exit, ret);

	if (tunables->time_in_single_enter >= tunables->single_enter_time)
		tunables->time_in_single_enter = 0;
	if (tunables->time_in_multi_enter >= tunables->multi_enter_time)
		tunables->time_in_multi_enter = 0;
	if (tunables->time_in_single_exit >= tunables->single_exit_time)
		tunables->time_in_single_exit = 0;
	if (tunables->time_in_multi_exit >= tunables->multi_exit_time)
		tunables->time_in_multi_exit = 0;
	tunables->mode_check_timestamp = now;

	return ret;
}

static void set_new_param_set(unsigned int index,
			struct cpufreq_interactive_tunables * tunables)
{
	unsigned long flags;

	tunables->hispeed_freq = tunables->hispeed_freq_set[index];
	tunables->go_hispeed_load = tunables->go_hispeed_load_set[index];
	tunables->min_sample_time = tunables->min_sample_time_set[index];
	tunables->timer_rate = tunables->timer_rate_set[index];

	spin_lock_irqsave(&tunables->target_loads_lock, flags);
	tunables->target_loads = tunables->target_loads_set[index];
	tunables->ntarget_loads = tunables->ntarget_loads_set[index];
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);
	tunables->above_hispeed_delay =
		tunables->above_hispeed_delay_set[index];
	tunables->nabove_hispeed_delay =
		tunables->nabove_hispeed_delay_set[index];
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	tunables->cur_param_index = index;
}

static void enter_mode(struct cpufreq_interactive_tunables * tunables)
{
	set_new_param_set(tunables->mode, tunables);
	if(!hmp_boost) {
		pr_debug("%s mp boost on", __func__);
		(void)set_hmp_boost(1);
		hmp_boost = true;
	}
}

static void exit_mode(struct cpufreq_interactive_tunables * tunables)
{
	set_new_param_set(0, tunables);
	if(hmp_boost) {
		pr_debug("%s mp boost off", __func__);
		(void)set_hmp_boost(0);
		hmp_boost = false;
	}
}
#endif

=======
	pcpu->time_in_idle = now_idle;
	pcpu->time_in_idle_timestamp = now;
	return now;
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static void cpufreq_interactive_timer(unsigned long data)
{
	u64 now;
	unsigned int delta_time;
	u64 cputime_speedadj;
	int cpu_load;
	struct cpufreq_interactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, data);
	struct cpufreq_interactive_tunables *tunables =
		pcpu->policy->governor_data;
	unsigned int new_freq;
	unsigned int loadadjfreq;
	unsigned int index;
	unsigned long flags;
<<<<<<< HEAD
	bool boosted;
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned int new_mode;
#endif
=======
	struct cpufreq_govinfo int_info;
	u64 max_fvtime;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (!down_read_trylock(&pcpu->enable_sem))
		return;
	if (!pcpu->governor_enabled)
		goto exit;

	spin_lock_irqsave(&pcpu->load_lock, flags);
<<<<<<< HEAD
	now = update_load(data);
	delta_time = (unsigned int)(now - pcpu->cputime_speedadj_timestamp);
	cputime_speedadj = pcpu->cputime_speedadj;
	spin_unlock_irqrestore(&pcpu->load_lock, flags);

	if (WARN_ON_ONCE(!delta_time))
		goto rearm;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->mode_lock, flags);
	if (tunables->enforced_mode)
		new_mode = tunables->enforced_mode;
	else
		new_mode = check_mode(data, tunables->mode, now);

	if (new_mode != tunables->mode) {
		tunables->mode = new_mode;
		if (new_mode & MULTI_MODE || new_mode & SINGLE_MODE)
			enter_mode(tunables);
		else
			exit_mode(tunables);
	}
	spin_unlock_irqrestore(&tunables->mode_lock, flags);
#endif
	do_div(cputime_speedadj, delta_time);
	loadadjfreq = (unsigned int)cputime_speedadj * 100;
	cpu_load = loadadjfreq / pcpu->target_freq;
	boosted = tunables->boost_val || now < tunables->boostpulse_endtime;

	if (cpu_load >= tunables->go_hispeed_load || boosted) {
		if (pcpu->target_freq < tunables->hispeed_freq) {
=======
	pcpu->last_evaluated_jiffy = get_jiffies_64();
	now = update_load(data);
	if (tunables->use_sched_load) {
		/*
		 * Unlock early to avoid deadlock.
		 *
		 * load_change_callback() for thread migration already
		 * holds rq lock. Then it locks load_lock to avoid racing
		 * with cpufreq_interactive_timer_resched/start().
		 * sched_get_busy() will also acquire rq lock. Thus we
		 * can't hold load_lock when calling sched_get_busy().
		 *
		 * load_lock used in this function protects time
		 * and load information. These stats are not used when
		 * scheduler input is available. Thus unlocking load_lock
		 * early is perfectly OK.
		 */
		spin_unlock_irqrestore(&pcpu->load_lock, flags);
		cputime_speedadj = (u64)sched_get_busy(data) *
				pcpu->policy->cpuinfo.max_freq;
		do_div(cputime_speedadj, tunables->timer_rate);
	} else {
		delta_time = (unsigned int)
				(now - pcpu->cputime_speedadj_timestamp);
		cputime_speedadj = pcpu->cputime_speedadj;
		spin_unlock_irqrestore(&pcpu->load_lock, flags);
		if (WARN_ON_ONCE(!delta_time))
			goto rearm;
		do_div(cputime_speedadj, delta_time);
	}

	loadadjfreq = (unsigned int)cputime_speedadj * 100;

	int_info.cpu = data;
	int_info.load = loadadjfreq / pcpu->policy->max;
	int_info.sampling_rate_us = tunables->timer_rate;
	atomic_notifier_call_chain(&cpufreq_govinfo_notifier_list,
					CPUFREQ_LOAD_CHANGE, &int_info);

	spin_lock_irqsave(&pcpu->target_freq_lock, flags);
	cpu_load = loadadjfreq / pcpu->policy->cur;
	tunables->boosted = tunables->boost_val || now < tunables->boostpulse_endtime;

	if (cpu_load >= tunables->go_hispeed_load || tunables->boosted) {
		if (pcpu->policy->cur < tunables->hispeed_freq) {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			new_freq = tunables->hispeed_freq;
		} else {
			new_freq = choose_freq(pcpu, loadadjfreq);

			if (new_freq < tunables->hispeed_freq)
				new_freq = tunables->hispeed_freq;
		}
	} else {
		new_freq = choose_freq(pcpu, loadadjfreq);
<<<<<<< HEAD
		if (new_freq > tunables->hispeed_freq &&
				pcpu->target_freq < tunables->hispeed_freq)
			new_freq = tunables->hispeed_freq;
	}

	if (pcpu->target_freq >= tunables->hispeed_freq &&
	    new_freq > pcpu->target_freq &&
	    now - pcpu->hispeed_validate_time <
	    freq_to_above_hispeed_delay(tunables, pcpu->target_freq)) {
		trace_cpufreq_interactive_notyet(
			data, cpu_load, pcpu->target_freq,
			pcpu->policy->cur, new_freq);
		goto rearm;
	}

	pcpu->hispeed_validate_time = now;

	if (cpufreq_frequency_table_target(pcpu->policy, pcpu->freq_table,
					   new_freq, CPUFREQ_RELATION_L,
					   &index))
		goto rearm;

	new_freq = pcpu->freq_table[index].frequency;

=======
	}

	if (pcpu->policy->cur >= tunables->hispeed_freq &&
	    new_freq > pcpu->policy->cur &&
	    now - pcpu->hispeed_validate_time <
	    freq_to_above_hispeed_delay(tunables, pcpu->policy->cur)) {
		trace_cpufreq_interactive_notyet(
			data, cpu_load, pcpu->target_freq,
			pcpu->policy->cur, new_freq);
		spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
		goto rearm;
	}

	pcpu->local_hvtime = now;

	if (cpufreq_frequency_table_target(pcpu->policy, pcpu->freq_table,
					   new_freq, CPUFREQ_RELATION_L,
					   &index)) {
		spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
		goto rearm;
	}

	new_freq = pcpu->freq_table[index].frequency;

	if (pcpu->target_freq >= pcpu->policy->max
	    && new_freq < pcpu->target_freq
	    && now - pcpu->max_freq_idle_start_time <
	    tunables->max_freq_hysteresis) {
		trace_cpufreq_interactive_notyet(data, cpu_load,
			pcpu->target_freq, pcpu->policy->cur, new_freq);
		spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
		goto rearm;
	}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * Do not scale below floor_freq unless we have been at or above the
	 * floor frequency for the minimum sample time since last validated.
	 */
<<<<<<< HEAD
	if (new_freq < pcpu->floor_freq) {
		if (now - pcpu->floor_validate_time <
				tunables->min_sample_time) {
			trace_cpufreq_interactive_notyet(
				data, cpu_load, pcpu->target_freq,
				pcpu->policy->cur, new_freq);
=======
	max_fvtime = max(pcpu->floor_validate_time, pcpu->local_fvtime);
	if (new_freq < pcpu->floor_freq &&
	    pcpu->target_freq >= pcpu->policy->cur) {
		if (now - max_fvtime < tunables->min_sample_time) {
			trace_cpufreq_interactive_notyet(
				data, cpu_load, pcpu->target_freq,
				pcpu->policy->cur, new_freq);
			spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			goto rearm;
		}
	}

	/*
	 * Update the timestamp for checking whether speed has been held at
	 * or above the selected frequency for a minimum of min_sample_time,
	 * if not boosted to hispeed_freq.  If boosted to hispeed_freq then we
	 * allow the speed to drop as soon as the boostpulse duration expires
	 * (or the indefinite boost is turned off).
	 */

<<<<<<< HEAD
	if (!boosted || new_freq > tunables->hispeed_freq) {
		pcpu->floor_freq = new_freq;
		pcpu->floor_validate_time = now;
	}

	if (pcpu->target_freq == new_freq &&
			pcpu->target_freq <= pcpu->policy->cur) {
		trace_cpufreq_interactive_already(
			data, cpu_load, pcpu->target_freq,
			pcpu->policy->cur, new_freq);
=======
	if (!tunables->boosted || new_freq > tunables->hispeed_freq) {
		pcpu->floor_freq = new_freq;
		if (pcpu->target_freq >= pcpu->policy->cur ||
		    new_freq >= pcpu->policy->cur)
			pcpu->local_fvtime = now;
	}

	if (pcpu->target_freq == new_freq) {
		trace_cpufreq_interactive_already(
			data, cpu_load, pcpu->target_freq,
			pcpu->policy->cur, new_freq);
		spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		goto rearm_if_notmax;
	}

	trace_cpufreq_interactive_target(data, cpu_load, pcpu->target_freq,
					 pcpu->policy->cur, new_freq);

	pcpu->target_freq = new_freq;
<<<<<<< HEAD
	spin_lock_irqsave(&speedchange_cpumask_lock, flags);
	cpumask_set_cpu(data, &speedchange_cpumask);
	spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);
	wake_up_process(tunables->speedchange_task);
=======
	spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
	spin_lock_irqsave(&speedchange_cpumask_lock, flags);
	cpumask_set_cpu(data, &speedchange_cpumask);
	spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);
	wake_up_process(speedchange_task);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

rearm_if_notmax:
	/*
	 * Already set max speed and don't see a need to change that,
	 * wait until next idle to re-evaluate, don't need timer.
	 */
	if (pcpu->target_freq == pcpu->policy->max)
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
		goto rearm;
#else
		goto exit;
#endif

rearm:
	if (!timer_pending(&pcpu->cpu_timer))
		cpufreq_interactive_timer_resched(pcpu);
=======
		goto exit;

rearm:
	if (!timer_pending(&pcpu->cpu_timer))
		cpufreq_interactive_timer_resched(data);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

exit:
	up_read(&pcpu->enable_sem);
	return;
}

static void cpufreq_interactive_idle_start(void)
{
	struct cpufreq_interactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, smp_processor_id());
	int pending;
<<<<<<< HEAD
=======
	struct cpufreq_interactive_tunables *tunables;
	unsigned long flags;
	u64 now;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (!down_read_trylock(&pcpu->enable_sem))
		return;
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		return;
	}

	pending = timer_pending(&pcpu->cpu_timer);

	if (pcpu->target_freq != pcpu->policy->min) {
		/*
		 * Entering idle while not at lowest speed.  On some
		 * platforms this can hold the other CPU(s) at that speed
		 * even though the CPU is idle. Set a timer to re-evaluate
		 * speed so this idle CPU doesn't hold the other CPUs above
		 * min indefinitely.  This should probably be a quirk of
		 * the CPUFreq driver.
		 */
<<<<<<< HEAD
		if (!pending)
			cpufreq_interactive_timer_resched(pcpu);
	}

=======
		if (!pending) {
			pcpu->last_evaluated_jiffy = get_jiffies_64();
			cpufreq_interactive_timer_resched(smp_processor_id());

			/*
			 * If timer is cancelled because CPU is running at
			 * policy->max, record the time CPU first goes to
			 * idle.
			 */
			now = ktime_to_us(ktime_get());
			tunables = pcpu->policy->governor_data;
			if (tunables->max_freq_hysteresis) {
				spin_lock_irqsave(&pcpu->target_freq_lock,
						  flags);
				pcpu->max_freq_idle_start_time = now;
				spin_unlock_irqrestore(&pcpu->target_freq_lock,
						       flags);
			}
		}
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	up_read(&pcpu->enable_sem);
}

static void cpufreq_interactive_idle_end(void)
{
	struct cpufreq_interactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, smp_processor_id());

	if (!down_read_trylock(&pcpu->enable_sem))
		return;
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		return;
	}

	/* Arm the timer for 1-2 ticks later if not already. */
	if (!timer_pending(&pcpu->cpu_timer)) {
<<<<<<< HEAD
		cpufreq_interactive_timer_resched(pcpu);
=======
		cpufreq_interactive_timer_resched(smp_processor_id());
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	} else if (time_after_eq(jiffies, pcpu->cpu_timer.expires)) {
		del_timer(&pcpu->cpu_timer);
		del_timer(&pcpu->cpu_slack_timer);
		cpufreq_interactive_timer(smp_processor_id());
	}

	up_read(&pcpu->enable_sem);
}

static int cpufreq_interactive_speedchange_task(void *data)
{
	unsigned int cpu;
	cpumask_t tmp_mask;
<<<<<<< HEAD
	cpumask_t policy_mask;
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	unsigned long flags;
	struct cpufreq_interactive_cpuinfo *pcpu;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&speedchange_cpumask_lock, flags);

<<<<<<< HEAD
		pcpu = &per_cpu(cpuinfo, smp_processor_id());
		cpumask_and(&policy_mask, &speedchange_cpumask, pcpu->policy->related_cpus);
		if (cpumask_empty(&policy_mask)) {
			spin_unlock_irqrestore(&speedchange_cpumask_lock,
					       flags);

			if (kthread_should_stop())
				break;

=======
		if (cpumask_empty(&speedchange_cpumask)) {
			spin_unlock_irqrestore(&speedchange_cpumask_lock,
					       flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&speedchange_cpumask_lock, flags);
		}

		set_current_state(TASK_RUNNING);
<<<<<<< HEAD
		cpumask_and(&tmp_mask, &speedchange_cpumask, pcpu->policy->related_cpus);
		cpumask_andnot(&speedchange_cpumask, &speedchange_cpumask, pcpu->policy->related_cpus);
=======
		tmp_mask = speedchange_cpumask;
		cpumask_clear(&speedchange_cpumask);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

		for_each_cpu(cpu, &tmp_mask) {
			unsigned int j;
			unsigned int max_freq = 0;
<<<<<<< HEAD

			pcpu = &per_cpu(cpuinfo, cpu);

=======
			struct cpufreq_interactive_cpuinfo *pjcpu;
			u64 hvt = ~0ULL, fvt = 0;

			pcpu = &per_cpu(cpuinfo, cpu);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			if (!down_read_trylock(&pcpu->enable_sem))
				continue;
			if (!pcpu->governor_enabled) {
				up_read(&pcpu->enable_sem);
				continue;
			}

			for_each_cpu(j, pcpu->policy->cpus) {
<<<<<<< HEAD
				struct cpufreq_interactive_cpuinfo *pjcpu =
					&per_cpu(cpuinfo, j);

				if (pjcpu->target_freq > max_freq)
					max_freq = pjcpu->target_freq;
			}

			if (max_freq != pcpu->policy->cur)
				__cpufreq_driver_target(pcpu->policy,
							max_freq,
							CPUFREQ_RELATION_H);

#if defined(CONFIG_CPU_THERMAL_IPA)
			ipa_cpufreq_requested(pcpu->policy, max_freq);
#endif

=======
				pjcpu = &per_cpu(cpuinfo, j);

				fvt = max(fvt, pjcpu->local_fvtime);
				if (pjcpu->target_freq > max_freq) {
					max_freq = pjcpu->target_freq;
					hvt = pjcpu->local_hvtime;
				} else if (pjcpu->target_freq == max_freq) {
					hvt = min(hvt, pjcpu->local_hvtime);
				}
			}
			for_each_cpu(j, pcpu->policy->cpus) {
				pjcpu = &per_cpu(cpuinfo, j);
				pjcpu->floor_validate_time = fvt;
			}

			if (max_freq != pcpu->policy->cur) {
				__cpufreq_driver_target(pcpu->policy,
							max_freq,
							CPUFREQ_RELATION_H);
				for_each_cpu(j, pcpu->policy->cpus) {
					pjcpu = &per_cpu(cpuinfo, j);
					pjcpu->hispeed_validate_time = hvt;
				}
			}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			trace_cpufreq_interactive_setspeed(cpu,
						     pcpu->target_freq,
						     pcpu->policy->cur);

			up_read(&pcpu->enable_sem);
		}
	}

	return 0;
}

<<<<<<< HEAD
static void cpufreq_interactive_boost(const struct cpufreq_policy *policy)
{
	int i;
	int anyboost = 0;
	unsigned long flags;
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_interactive_tunables *tunables;
	struct cpumask boost_mask;

	spin_lock_irqsave(&speedchange_cpumask_lock, flags);

	if (have_governor_per_policy())
		cpumask_copy(&boost_mask, policy->cpus);
	else
		cpumask_copy(&boost_mask, cpu_online_mask);

	for_each_cpu(i, &boost_mask) {
		pcpu = &per_cpu(cpuinfo, i);
		tunables = pcpu->policy->governor_data;

		if (!tunables->speedchange_task) {
			spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);
			return;
		}

=======
static void cpufreq_interactive_boost(struct cpufreq_interactive_tunables *tunables)
{
	int i;
	int anyboost = 0;
	unsigned long flags[2];
	struct cpufreq_interactive_cpuinfo *pcpu;

	tunables->boosted = true;

	spin_lock_irqsave(&speedchange_cpumask_lock, flags[0]);

	for_each_online_cpu(i) {
		pcpu = &per_cpu(cpuinfo, i);
		if (tunables != pcpu->policy->governor_data)
			continue;

		spin_lock_irqsave(&pcpu->target_freq_lock, flags[1]);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (pcpu->target_freq < tunables->hispeed_freq) {
			pcpu->target_freq = tunables->hispeed_freq;
			cpumask_set_cpu(i, &speedchange_cpumask);
			pcpu->hispeed_validate_time =
				ktime_to_us(ktime_get());
			anyboost = 1;
		}

		/*
		 * Set floor freq and (re)start timer for when last
		 * validated.
		 */

		pcpu->floor_freq = tunables->hispeed_freq;
		pcpu->floor_validate_time = ktime_to_us(ktime_get());
<<<<<<< HEAD
	}

	spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

	if (anyboost && tunables->speedchange_task)
		wake_up_process(tunables->speedchange_task);
}

=======
		spin_unlock_irqrestore(&pcpu->target_freq_lock, flags[1]);
	}

	spin_unlock_irqrestore(&speedchange_cpumask_lock, flags[0]);

	if (anyboost)
		wake_up_process(speedchange_task);
}

static int load_change_callback(struct notifier_block *nb, unsigned long val,
				void *data)
{
	unsigned long cpu = (unsigned long) data;
	struct cpufreq_interactive_cpuinfo *pcpu = &per_cpu(cpuinfo, cpu);
	struct cpufreq_interactive_tunables *tunables;

	if (speedchange_task == current)
		return 0;

	if (!down_read_trylock(&pcpu->enable_sem))
		return 0;
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		return 0;
	}
	tunables = pcpu->policy->governor_data;
	if (!tunables->use_sched_load || !tunables->use_migration_notif) {
		up_read(&pcpu->enable_sem);
		return 0;
	}

	trace_cpufreq_interactive_load_change(cpu);
	del_timer(&pcpu->cpu_timer);
	del_timer(&pcpu->cpu_slack_timer);
	cpufreq_interactive_timer(cpu);

	up_read(&pcpu->enable_sem);
	return 0;
}

static struct notifier_block load_notifier_block = {
	.notifier_call = load_change_callback,
};

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static int cpufreq_interactive_notifier(
	struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	struct cpufreq_interactive_cpuinfo *pcpu;
	int cpu;
	unsigned long flags;

	if (val == CPUFREQ_POSTCHANGE) {
		pcpu = &per_cpu(cpuinfo, freq->cpu);
		if (!down_read_trylock(&pcpu->enable_sem))
			return 0;
		if (!pcpu->governor_enabled) {
			up_read(&pcpu->enable_sem);
			return 0;
		}

		for_each_cpu(cpu, pcpu->policy->cpus) {
			struct cpufreq_interactive_cpuinfo *pjcpu =
				&per_cpu(cpuinfo, cpu);
			if (cpu != freq->cpu) {
				if (!down_read_trylock(&pjcpu->enable_sem))
					continue;
				if (!pjcpu->governor_enabled) {
					up_read(&pjcpu->enable_sem);
					continue;
				}
			}
			spin_lock_irqsave(&pjcpu->load_lock, flags);
			update_load(cpu);
			spin_unlock_irqrestore(&pjcpu->load_lock, flags);
			if (cpu != freq->cpu)
				up_read(&pjcpu->enable_sem);
		}

		up_read(&pcpu->enable_sem);
	}
	return 0;
}

static struct notifier_block cpufreq_notifier_block = {
	.notifier_call = cpufreq_interactive_notifier,
};

static unsigned int *get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp;
	int i;
	int ntokens = 1;
	unsigned int *tokenized_data;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	if (!(ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf(cp, "%u", &tokenized_data[i++]) != 1)
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_kfree;

	*num_tokens = ntokens;
	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}

static ssize_t show_target_loads(
	struct cpufreq_interactive_tunables *tunables,
	char *buf)
{
	int i;
	ssize_t ret = 0;
	unsigned long flags;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif

	spin_lock_irqsave(&tunables->target_loads_lock, flags);
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	for (i = 0; i < tunables->ntarget_loads_set[tunables->param_index]; i++)
		ret += sprintf(buf + ret, "%u%s",
			tunables->target_loads_set[tunables->param_index][i],
			i & 0x1 ? ":" : " ");
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
	for (i = 0; i < tunables->ntarget_loads; i++)
		ret += sprintf(buf + ret, "%u%s", tunables->target_loads[i],
			       i & 0x1 ? ":" : " ");
#endif
=======

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads; i++)
		ret += sprintf(buf + ret, "%u%s", tunables->target_loads[i],
			       i & 0x1 ? ":" : " ");

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	sprintf(buf + ret - 1, "\n");
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);
	return ret;
}

static ssize_t store_target_loads(
	struct cpufreq_interactive_tunables *tunables,
	const char *buf, size_t count)
{
	int ntokens;
	unsigned int *new_target_loads = NULL;
	unsigned long flags;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	new_target_loads = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_target_loads))
		return PTR_RET(new_target_loads);

	spin_lock_irqsave(&tunables->target_loads_lock, flags);
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	if (tunables->target_loads_set[tunables->param_index] != default_target_loads)
		kfree(tunables->target_loads_set[tunables->param_index]);
	tunables->target_loads_set[tunables->param_index] = new_target_loads;
	tunables->ntarget_loads_set[tunables->param_index] = ntokens;
	if (tunables->cur_param_index == tunables->param_index) {
		tunables->target_loads = new_target_loads;
		tunables->ntarget_loads = ntokens;
	}
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (tunables->target_loads != default_target_loads)
		kfree(tunables->target_loads);
	tunables->target_loads = new_target_loads;
	tunables->ntarget_loads = ntokens;
<<<<<<< HEAD
#endif
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

=======
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return count;
}

static ssize_t show_above_hispeed_delay(
	struct cpufreq_interactive_tunables *tunables, char *buf)
{
	int i;
	ssize_t ret = 0;
	unsigned long flags;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	for (i = 0; i < tunables->nabove_hispeed_delay_set[tunables->param_index]; i++)
		ret += sprintf(buf + ret, "%u%s",
			tunables->above_hispeed_delay_set[tunables->param_index][i],
			i & 0x1 ? ":" : " ");
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
=======

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	for (i = 0; i < tunables->nabove_hispeed_delay; i++)
		ret += sprintf(buf + ret, "%u%s",
			       tunables->above_hispeed_delay[i],
			       i & 0x1 ? ":" : " ");
<<<<<<< HEAD
#endif
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	sprintf(buf + ret - 1, "\n");
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);
	return ret;
}

static ssize_t store_above_hispeed_delay(
	struct cpufreq_interactive_tunables *tunables,
	const char *buf, size_t count)
{
	int ntokens;
	unsigned int *new_above_hispeed_delay = NULL;
	unsigned long flags;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	new_above_hispeed_delay = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_above_hispeed_delay))
		return PTR_RET(new_above_hispeed_delay);

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	if (tunables->above_hispeed_delay_set[tunables->param_index] != default_above_hispeed_delay)
		kfree(tunables->above_hispeed_delay_set[tunables->param_index]);
	tunables->above_hispeed_delay_set[tunables->param_index] = new_above_hispeed_delay;
	tunables->nabove_hispeed_delay_set[tunables->param_index] = ntokens;
	if (tunables->cur_param_index == tunables->param_index) {
		tunables->above_hispeed_delay = new_above_hispeed_delay;
		tunables->nabove_hispeed_delay = ntokens;
	}
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (tunables->above_hispeed_delay != default_above_hispeed_delay)
		kfree(tunables->above_hispeed_delay);
	tunables->above_hispeed_delay = new_above_hispeed_delay;
	tunables->nabove_hispeed_delay = ntokens;
<<<<<<< HEAD
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);
	return count;

}

static ssize_t show_hispeed_freq(struct cpufreq_interactive_tunables *tunables,
		char *buf)
{
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	return sprintf(buf, "%u\n", tunables->hispeed_freq_set[tunables->param_index]);
#else
	return sprintf(buf, "%u\n", tunables->hispeed_freq);
#endif
=======
	return sprintf(buf, "%u\n", tunables->hispeed_freq);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static ssize_t store_hispeed_freq(struct cpufreq_interactive_tunables *tunables,
		const char *buf, size_t count)
{
	int ret;
	long unsigned int val;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	tunables->hispeed_freq_set[tunables->param_index] = val;
	if (tunables->cur_param_index == tunables->param_index)
		tunables->hispeed_freq = val;
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
	tunables->hispeed_freq = val;
#endif
	return count;
}

static ssize_t show_go_hispeed_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
#ifdef CONFIG_MODE_AUTO_CHANGE
	return sprintf(buf, "%lu\n", tunables->go_hispeed_load_set[tunables->param_index]);
#else
	return sprintf(buf, "%lu\n", tunables->go_hispeed_load);
#endif
=======

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->hispeed_freq = val;
	return count;
}

#define show_store_one(file_name)					\
static ssize_t show_##file_name(					\
	struct cpufreq_interactive_tunables *tunables, char *buf)	\
{									\
	return snprintf(buf, PAGE_SIZE, "%u\n", tunables->file_name);	\
}									\
static ssize_t store_##file_name(					\
		struct cpufreq_interactive_tunables *tunables,		\
		const char *buf, size_t count)				\
{									\
	int ret;							\
	long unsigned int val;						\
									\
	ret = kstrtoul(buf, 0, &val);				\
	if (ret < 0)							\
		return ret;						\
	tunables->file_name = val;					\
	return count;							\
}
show_store_one(max_freq_hysteresis);
show_store_one(align_windows);

static ssize_t show_go_hispeed_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->go_hispeed_load);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static ssize_t store_go_hispeed_load(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	unsigned long val;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	tunables->go_hispeed_load_set[tunables->param_index] = val;
	if (tunables->cur_param_index == tunables->param_index)
		tunables->go_hispeed_load = val;
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
	tunables->go_hispeed_load = val;
#endif
=======

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->go_hispeed_load = val;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return count;
}

static ssize_t show_min_sample_time(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	return sprintf(buf, "%lu\n", tunables->min_sample_time_set[tunables->param_index]);
#else
	return sprintf(buf, "%lu\n", tunables->min_sample_time);
#endif
=======
	return sprintf(buf, "%lu\n", tunables->min_sample_time);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static ssize_t store_min_sample_time(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	unsigned long val;
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	tunables->min_sample_time_set[tunables->param_index] = val;
	if (tunables->cur_param_index == tunables->param_index)
		tunables->min_sample_time = val;
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
	tunables->min_sample_time = val;
#endif
=======

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->min_sample_time = val;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return count;
}

static ssize_t show_timer_rate(struct cpufreq_interactive_tunables *tunables,
		char *buf)
{
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	return sprintf(buf, "%lu\n", tunables->timer_rate_set[tunables->param_index]);
#else
	return sprintf(buf, "%lu\n", tunables->timer_rate);
#endif
=======
	return sprintf(buf, "%lu\n", tunables->timer_rate);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

static ssize_t store_timer_rate(struct cpufreq_interactive_tunables *tunables,
		const char *buf, size_t count)
{
	int ret;
<<<<<<< HEAD
	unsigned long val;
#ifdef CONFIG_MODE_AUTO_CHANGE
	unsigned long flags_idx;
#endif
	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
#ifdef CONFIG_MODE_AUTO_CHANGE
	spin_lock_irqsave(&tunables->param_index_lock, flags_idx);
	tunables->timer_rate_set[tunables->param_index] = val;
	if (tunables->cur_param_index == tunables->param_index)
		tunables->timer_rate = val;
	spin_unlock_irqrestore(&tunables->param_index_lock, flags_idx);
#else
	tunables->timer_rate = val;
#endif
=======
	unsigned long val, val_round;
	struct cpufreq_interactive_tunables *t;
	int cpu;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	val_round = jiffies_to_usecs(usecs_to_jiffies(val));
	if (val != val_round)
		pr_warn("timer_rate not aligned to jiffy. Rounded up to %lu\n",
			val_round);
	tunables->timer_rate = val_round;

	if (!tunables->use_sched_load)
		return count;

	for_each_possible_cpu(cpu) {
		t = per_cpu(cpuinfo, cpu).cached_tunables;
		if (t && t->use_sched_load)
			t->timer_rate = val_round;
	}
	set_window_helper(tunables);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return count;
}

static ssize_t show_timer_slack(struct cpufreq_interactive_tunables *tunables,
		char *buf)
{
	return sprintf(buf, "%d\n", tunables->timer_slack_val);
}

static ssize_t store_timer_slack(struct cpufreq_interactive_tunables *tunables,
		const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	tunables->timer_slack_val = val;
	return count;
}

static ssize_t show_boost(struct cpufreq_interactive_tunables *tunables,
			  char *buf)
{
	return sprintf(buf, "%d\n", tunables->boost_val);
}

static ssize_t store_boost(struct cpufreq_interactive_tunables *tunables,
			   const char *buf, size_t count)
{
	int ret;
	unsigned long val;
<<<<<<< HEAD
	struct cpufreq_policy *policy = container_of(tunables->policy,
						struct cpufreq_policy, policy);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	tunables->boost_val = val;

	if (tunables->boost_val) {
		trace_cpufreq_interactive_boost("on");
<<<<<<< HEAD
		cpufreq_interactive_boost(policy);
	} else {
=======
		if (!tunables->boosted)
			cpufreq_interactive_boost(tunables);
	} else {
		tunables->boostpulse_endtime = ktime_to_us(ktime_get());
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		trace_cpufreq_interactive_unboost("off");
	}

	return count;
}

static ssize_t store_boostpulse(struct cpufreq_interactive_tunables *tunables,
				const char *buf, size_t count)
{
	int ret;
	unsigned long val;
<<<<<<< HEAD
	struct cpufreq_policy *policy = container_of(tunables->policy,
						struct cpufreq_policy, policy);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	tunables->boostpulse_endtime = ktime_to_us(ktime_get()) +
		tunables->boostpulse_duration_val;
	trace_cpufreq_interactive_boost("pulse");
<<<<<<< HEAD
	cpufreq_interactive_boost(policy);
=======
	if (!tunables->boosted)
		cpufreq_interactive_boost(tunables);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return count;
}

static ssize_t show_boostpulse_duration(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->boostpulse_duration_val);
}

static ssize_t store_boostpulse_duration(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	tunables->boostpulse_duration_val = val;
	return count;
}

static ssize_t show_io_is_busy(struct cpufreq_interactive_tunables *tunables,
		char *buf)
{
	return sprintf(buf, "%u\n", tunables->io_is_busy);
}

static ssize_t store_io_is_busy(struct cpufreq_interactive_tunables *tunables,
		const char *buf, size_t count)
{
	int ret;
	unsigned long val;
<<<<<<< HEAD
=======
	struct cpufreq_interactive_tunables *t;
	int cpu;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;
	tunables->io_is_busy = val;
<<<<<<< HEAD
	return count;
}

#ifdef CONFIG_MODE_AUTO_CHANGE
static ssize_t show_mode(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->mode);
}

static ssize_t store_mode(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	val &= MULTI_MODE | SINGLE_MODE | NO_MODE;
	tunables->mode = val;
	return count;
}

static ssize_t show_enforced_mode(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->enforced_mode);
}

static ssize_t store_enforced_mode(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	val &= MULTI_MODE | SINGLE_MODE | NO_MODE;
	tunables->enforced_mode = val;
	return count;
}

static ssize_t show_param_index(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->param_index);
}

static ssize_t store_param_index(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;
	unsigned long flags;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	val &= MULTI_MODE | SINGLE_MODE | NO_MODE;

	spin_lock_irqsave(&tunables->param_index_lock, flags);
	tunables->param_index = val;
	spin_unlock_irqrestore(&tunables->param_index_lock, flags);
	return count;
}

static ssize_t show_multi_enter_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->multi_enter_load);
}

static ssize_t store_multi_enter_load(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->multi_enter_load = val;
	return count;
}

static ssize_t show_multi_exit_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->multi_exit_load);
}

static ssize_t store_multi_exit_load(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->multi_exit_load = val;
	return count;
}

static ssize_t show_single_enter_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->single_enter_load);
}

static ssize_t store_single_enter_load(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->single_enter_load = val;
	return count;
}

static ssize_t show_single_exit_load(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%d\n", tunables->single_exit_load);
}

static ssize_t store_single_exit_load(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->single_exit_load = val;
	return count;
}

static ssize_t show_multi_enter_time(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->multi_enter_time);
}

static ssize_t store_multi_enter_time(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->multi_enter_time = val;
	return count;
}

static ssize_t show_multi_exit_time(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->multi_exit_time);
}

static ssize_t store_multi_exit_time(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->multi_exit_time = val;
	return count;
}

static ssize_t show_single_enter_time(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->single_enter_time);
}

static ssize_t store_single_enter_time(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->single_enter_time = val;
	return count;
}

static ssize_t show_single_exit_time(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	return sprintf(buf, "%lu\n", tunables->single_exit_time);
}

static ssize_t store_single_exit_time(struct cpufreq_interactive_tunables
		*tunables, const char *buf, size_t count)
{
	int ret;
	long unsigned int val;

	ret = strict_strtoul(buf, 0, &val);
	if (ret < 0)
			return ret;

	tunables->single_exit_time = val;
	return count;
}

static ssize_t show_cpu_util(struct cpufreq_interactive_tunables
		*tunables, char *buf)
{
	int i;
	u64 now;
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_policy *policy = container_of(tunables->policy,
		struct cpufreq_policy, policy);
	ssize_t ret = 0;

	for_each_cpu_mask(i, policy->related_cpus[0]) {
		if (cpu_online(i)) {
			pcpu = &per_cpu(cpuinfo, i);
			get_cpu_idle_time(i, &now, tunables->io_is_busy);

			if (now - pcpu->time_in_idle_timestamp <= tunables->timer_rate)
				ret += sprintf(buf + ret, "%3u ", per_cpu(cpu_util, i));
			else
				ret += sprintf(buf + ret, "%3s ", (pcpu->target_freq == pcpu->policy->max) ? "H_I" : "L_I");
		} else
			ret += sprintf(buf + ret, "OFF ");
	}

	ret += sprintf(buf + --ret, "\n");
	return ret;
}
#endif
=======

	if (!tunables->use_sched_load)
		return count;

	for_each_possible_cpu(cpu) {
		t = per_cpu(cpuinfo, cpu).cached_tunables;
		if (t && t->use_sched_load)
			t->io_is_busy = val;
	}
	sched_set_io_is_busy(val);

	return count;
}

static int cpufreq_interactive_enable_sched_input(
			struct cpufreq_interactive_tunables *tunables)
{
	int rc = 0, j;
	struct cpufreq_interactive_tunables *t;

	mutex_lock(&sched_lock);

	set_window_count++;
	if (set_window_count != 1) {
		for_each_possible_cpu(j) {
			t = per_cpu(cpuinfo, j).cached_tunables;
			if (t && t->use_sched_load) {
				tunables->timer_rate = t->timer_rate;
				tunables->io_is_busy = t->io_is_busy;
				break;
			}
		}
		goto out;
	}

	rc = set_window_helper(tunables);
	if (rc) {
		pr_err("%s: Failed to set sched window\n", __func__);
		set_window_count--;
		goto out;
	}
	sched_set_io_is_busy(tunables->io_is_busy);

	if (!tunables->use_migration_notif)
		goto out;

	migration_register_count++;
	if (migration_register_count != 1)
		goto out;
	else
		atomic_notifier_chain_register(&load_alert_notifier_head,
						&load_notifier_block);
out:
	mutex_unlock(&sched_lock);
	return rc;
}

static int cpufreq_interactive_disable_sched_input(
			struct cpufreq_interactive_tunables *tunables)
{
	mutex_lock(&sched_lock);

	if (tunables->use_migration_notif) {
		migration_register_count--;
		if (!migration_register_count)
			atomic_notifier_chain_unregister(
					&load_alert_notifier_head,
					&load_notifier_block);
	}
	set_window_count--;

	mutex_unlock(&sched_lock);
	return 0;
}

static ssize_t show_use_sched_load(
		struct cpufreq_interactive_tunables *tunables, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", tunables->use_sched_load);
}

static ssize_t store_use_sched_load(
			struct cpufreq_interactive_tunables *tunables,
			const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	if (tunables->use_sched_load == (bool) val)
		return count;
	if (val)
		ret = cpufreq_interactive_enable_sched_input(tunables);
	else
		ret = cpufreq_interactive_disable_sched_input(tunables);

	if (ret)
		return ret;

	tunables->use_sched_load = val;
	return count;
}

static ssize_t show_use_migration_notif(
		struct cpufreq_interactive_tunables *tunables, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n",
			tunables->use_migration_notif);
}

static ssize_t store_use_migration_notif(
			struct cpufreq_interactive_tunables *tunables,
			const char *buf, size_t count)
{
	int ret;
	unsigned long val;

	ret = kstrtoul(buf, 0, &val);
	if (ret < 0)
		return ret;

	if (tunables->use_migration_notif == (bool) val)
		return count;
	tunables->use_migration_notif = val;

	if (!tunables->use_sched_load)
		return count;

	mutex_lock(&sched_lock);
	if (val) {
		migration_register_count++;
		if (migration_register_count == 1)
			atomic_notifier_chain_register(
					&load_alert_notifier_head,
					&load_notifier_block);
	} else {
		migration_register_count--;
		if (!migration_register_count)
			atomic_notifier_chain_unregister(
					&load_alert_notifier_head,
					&load_notifier_block);
	}
	mutex_unlock(&sched_lock);

	return count;
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/*
 * Create show/store routines
 * - sys: One governor instance for complete SYSTEM
 * - pol: One governor instance per struct cpufreq_policy
 */
#define show_gov_pol_sys(file_name)					\
static ssize_t show_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return show_##file_name(common_tunables, buf);			\
}									\
									\
static ssize_t show_##file_name##_gov_pol				\
(struct cpufreq_policy *policy, char *buf)				\
{									\
	return show_##file_name(policy->governor_data, buf);		\
}

#define store_gov_pol_sys(file_name)					\
static ssize_t store_##file_name##_gov_sys				\
(struct kobject *kobj, struct attribute *attr, const char *buf,		\
	size_t count)							\
{									\
	return store_##file_name(common_tunables, buf, count);		\
}									\
									\
static ssize_t store_##file_name##_gov_pol				\
(struct cpufreq_policy *policy, const char *buf, size_t count)		\
{									\
	return store_##file_name(policy->governor_data, buf, count);	\
}

#define show_store_gov_pol_sys(file_name)				\
show_gov_pol_sys(file_name);						\
store_gov_pol_sys(file_name)

show_store_gov_pol_sys(target_loads);
show_store_gov_pol_sys(above_hispeed_delay);
show_store_gov_pol_sys(hispeed_freq);
show_store_gov_pol_sys(go_hispeed_load);
show_store_gov_pol_sys(min_sample_time);
show_store_gov_pol_sys(timer_rate);
show_store_gov_pol_sys(timer_slack);
show_store_gov_pol_sys(boost);
store_gov_pol_sys(boostpulse);
show_store_gov_pol_sys(boostpulse_duration);
show_store_gov_pol_sys(io_is_busy);
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
show_store_gov_pol_sys(mode);
show_store_gov_pol_sys(enforced_mode);
show_store_gov_pol_sys(param_index);
show_store_gov_pol_sys(multi_enter_load);
show_store_gov_pol_sys(multi_exit_load);
show_store_gov_pol_sys(single_enter_load);
show_store_gov_pol_sys(single_exit_load);
show_store_gov_pol_sys(multi_enter_time);
show_store_gov_pol_sys(multi_exit_time);
show_store_gov_pol_sys(single_enter_time);
show_store_gov_pol_sys(single_exit_time);
show_gov_pol_sys(cpu_util);
#endif
#define gov_sys_attr_rw(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0660, show_##_name##_gov_sys, store_##_name##_gov_sys)

#define gov_pol_attr_rw(_name)						\
static struct freq_attr _name##_gov_pol =				\
__ATTR(_name, 0660, show_##_name##_gov_pol, store_##_name##_gov_pol)
=======
show_store_gov_pol_sys(use_sched_load);
show_store_gov_pol_sys(use_migration_notif);
show_store_gov_pol_sys(max_freq_hysteresis);
show_store_gov_pol_sys(align_windows);

#define gov_sys_attr_rw(_name)						\
static struct global_attr _name##_gov_sys =				\
__ATTR(_name, 0644, show_##_name##_gov_sys, store_##_name##_gov_sys)

#define gov_pol_attr_rw(_name)						\
static struct freq_attr _name##_gov_pol =				\
__ATTR(_name, 0644, show_##_name##_gov_pol, store_##_name##_gov_pol)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#define gov_sys_pol_attr_rw(_name)					\
	gov_sys_attr_rw(_name);						\
	gov_pol_attr_rw(_name)

gov_sys_pol_attr_rw(target_loads);
gov_sys_pol_attr_rw(above_hispeed_delay);
gov_sys_pol_attr_rw(hispeed_freq);
gov_sys_pol_attr_rw(go_hispeed_load);
gov_sys_pol_attr_rw(min_sample_time);
gov_sys_pol_attr_rw(timer_rate);
gov_sys_pol_attr_rw(timer_slack);
gov_sys_pol_attr_rw(boost);
gov_sys_pol_attr_rw(boostpulse_duration);
gov_sys_pol_attr_rw(io_is_busy);
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
gov_sys_pol_attr_rw(mode);
gov_sys_pol_attr_rw(enforced_mode);
gov_sys_pol_attr_rw(param_index);
gov_sys_pol_attr_rw(multi_enter_load);
gov_sys_pol_attr_rw(multi_exit_load);
gov_sys_pol_attr_rw(single_enter_load);
gov_sys_pol_attr_rw(single_exit_load);
gov_sys_pol_attr_rw(multi_enter_time);
gov_sys_pol_attr_rw(multi_exit_time);
gov_sys_pol_attr_rw(single_enter_time);
gov_sys_pol_attr_rw(single_exit_time);
#endif
=======
gov_sys_pol_attr_rw(use_sched_load);
gov_sys_pol_attr_rw(use_migration_notif);
gov_sys_pol_attr_rw(max_freq_hysteresis);
gov_sys_pol_attr_rw(align_windows);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

static struct global_attr boostpulse_gov_sys =
	__ATTR(boostpulse, 0200, NULL, store_boostpulse_gov_sys);

static struct freq_attr boostpulse_gov_pol =
	__ATTR(boostpulse, 0200, NULL, store_boostpulse_gov_pol);
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
static struct global_attr cpu_util_gov_sys =
	__ATTR(cpu_util, 0444, show_cpu_util_gov_sys, NULL);

static struct freq_attr cpu_util_gov_pol =
	__ATTR(cpu_util, 0444, show_cpu_util_gov_pol, NULL);
#endif
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/* One Governor instance for entire system */
static struct attribute *interactive_attributes_gov_sys[] = {
	&target_loads_gov_sys.attr,
	&above_hispeed_delay_gov_sys.attr,
	&hispeed_freq_gov_sys.attr,
	&go_hispeed_load_gov_sys.attr,
	&min_sample_time_gov_sys.attr,
	&timer_rate_gov_sys.attr,
	&timer_slack_gov_sys.attr,
	&boost_gov_sys.attr,
	&boostpulse_gov_sys.attr,
	&boostpulse_duration_gov_sys.attr,
	&io_is_busy_gov_sys.attr,
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	&mode_gov_sys.attr,
	&enforced_mode_gov_sys.attr,
	&param_index_gov_sys.attr,
	&multi_enter_load_gov_sys.attr,
	&multi_exit_load_gov_sys.attr,
	&single_enter_load_gov_sys.attr,
	&single_exit_load_gov_sys.attr,
	&multi_enter_time_gov_sys.attr,
	&multi_exit_time_gov_sys.attr,
	&single_enter_time_gov_sys.attr,
	&single_exit_time_gov_sys.attr,
	&cpu_util_gov_sys.attr,
#endif
=======
	&use_sched_load_gov_sys.attr,
	&use_migration_notif_gov_sys.attr,
	&max_freq_hysteresis_gov_sys.attr,
	&align_windows_gov_sys.attr,
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	NULL,
};

static struct attribute_group interactive_attr_group_gov_sys = {
	.attrs = interactive_attributes_gov_sys,
	.name = "interactive",
};

/* Per policy governor instance */
static struct attribute *interactive_attributes_gov_pol[] = {
	&target_loads_gov_pol.attr,
	&above_hispeed_delay_gov_pol.attr,
	&hispeed_freq_gov_pol.attr,
	&go_hispeed_load_gov_pol.attr,
	&min_sample_time_gov_pol.attr,
	&timer_rate_gov_pol.attr,
	&timer_slack_gov_pol.attr,
	&boost_gov_pol.attr,
	&boostpulse_gov_pol.attr,
	&boostpulse_duration_gov_pol.attr,
	&io_is_busy_gov_pol.attr,
<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
	&mode_gov_pol.attr,
	&enforced_mode_gov_pol.attr,
	&param_index_gov_pol.attr,
	&multi_enter_load_gov_pol.attr,
	&multi_exit_load_gov_pol.attr,
	&single_enter_load_gov_pol.attr,
	&single_exit_load_gov_pol.attr,
	&multi_enter_time_gov_pol.attr,
	&multi_exit_time_gov_pol.attr,
	&single_enter_time_gov_pol.attr,
	&single_exit_time_gov_pol.attr,
	&cpu_util_gov_pol.attr,
#endif
=======
	&use_sched_load_gov_pol.attr,
	&use_migration_notif_gov_pol.attr,
	&max_freq_hysteresis_gov_pol.attr,
	&align_windows_gov_pol.attr,
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	NULL,
};

static struct attribute_group interactive_attr_group_gov_pol = {
	.attrs = interactive_attributes_gov_pol,
	.name = "interactive",
};

<<<<<<< HEAD
#ifdef CONFIG_ANDROID
static const char *interactive_sysfs[] = {
	"target_loads",
	"above_hispeed_delay",
	"hispeed_freq",
	"go_hispeed_load",
	"min_sample_time",
	"timer_rate",
	"timer_slack",
	"boost",
	"boostpulse",
	"boostpulse_duration",
	"io_is_busy",
#ifdef CONFIG_MODE_AUTO_CHANGE
	"mode",
	"enforced_mode",
	"param_index",
	"multi_enter_load",
	"multi_exit_load",
	"single_enter_load",
	"single_exit_load",
	"multi_enter_time",
	"multi_exit_time",
	"single_enter_time",
	"single_exit_time",
	"cpu_util",
#endif
};
#endif

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static struct attribute_group *get_sysfs_attr(void)
{
	if (have_governor_per_policy())
		return &interactive_attr_group_gov_pol;
	else
		return &interactive_attr_group_gov_sys;
}

static int cpufreq_interactive_idle_notifier(struct notifier_block *nb,
					     unsigned long val,
					     void *data)
{
	switch (val) {
	case IDLE_START:
		cpufreq_interactive_idle_start();
		break;
	case IDLE_END:
		cpufreq_interactive_idle_end();
		break;
	}

	return 0;
}

static struct notifier_block cpufreq_interactive_idle_nb = {
	.notifier_call = cpufreq_interactive_idle_notifier,
};

<<<<<<< HEAD
#ifdef CONFIG_ANDROID
static void change_sysfs_owner(struct cpufreq_policy *policy)
{
	char buf[NAME_MAX];
	mm_segment_t oldfs;
	int i;
	char *path = kobject_get_path(get_governor_parent_kobj(policy),
			GFP_KERNEL);

	oldfs = get_fs();
	set_fs(get_ds());

	for (i = 0; i < ARRAY_SIZE(interactive_sysfs); i++) {
		snprintf(buf, sizeof(buf), "/sys%s/interactive/%s", path,
				interactive_sysfs[i]);
		sys_chown(buf, AID_SYSTEM, AID_SYSTEM);
	}

	set_fs(oldfs);
	kfree(path);
}
#else
static inline void change_sysfs_owner(struct cpufreq_policy *policy) { }
#endif

#ifdef CONFIG_MODE_AUTO_CHANGE
static void cpufreq_param_set_init(struct cpufreq_interactive_tunables *tunables)
{
	unsigned int i;

	tunables->multi_enter_load = DEFAULT_TARGET_LOAD * num_possible_cpus() / 2;

	for (i = 0; i < MAX_PARAM_SET; i++) {
		tunables->hispeed_freq_set[i] = 0;
		tunables->go_hispeed_load_set[i] = tunables->go_hispeed_load;
		tunables->target_loads_set[i] = tunables->target_loads;
		tunables->ntarget_loads_set[i] = tunables->ntarget_loads;
		tunables->min_sample_time_set[i] = tunables->min_sample_time;
		tunables->timer_rate_set[i] = tunables->timer_rate;
		tunables->above_hispeed_delay_set[i] = tunables->above_hispeed_delay;
		tunables->nabove_hispeed_delay_set[i] = tunables->nabove_hispeed_delay;
		tunables->sampling_down_factor_set[i] = tunables->sampling_down_factor;
	}
}
#endif
=======
static void save_tunables(struct cpufreq_policy *policy,
			  struct cpufreq_interactive_tunables *tunables)
{
	int cpu;
	struct cpufreq_interactive_cpuinfo *pcpu;

	if (have_governor_per_policy())
		cpu = cpumask_first(policy->related_cpus);
	else
		cpu = 0;

	pcpu = &per_cpu(cpuinfo, cpu);
	WARN_ON(pcpu->cached_tunables && pcpu->cached_tunables != tunables);
	pcpu->cached_tunables = tunables;
}

static struct cpufreq_interactive_tunables *alloc_tunable(
					struct cpufreq_policy *policy)
{
	struct cpufreq_interactive_tunables *tunables;

	tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
	if (!tunables) {
		pr_err("%s: POLICY_INIT: kzalloc failed\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	tunables->above_hispeed_delay = default_above_hispeed_delay;
	tunables->nabove_hispeed_delay =
		ARRAY_SIZE(default_above_hispeed_delay);
	tunables->go_hispeed_load = DEFAULT_GO_HISPEED_LOAD;
	tunables->target_loads = default_target_loads;
	tunables->ntarget_loads = ARRAY_SIZE(default_target_loads);
	tunables->min_sample_time = DEFAULT_MIN_SAMPLE_TIME;
	tunables->timer_rate = DEFAULT_TIMER_RATE;
	tunables->boostpulse_duration_val = DEFAULT_MIN_SAMPLE_TIME;
	tunables->timer_slack_val = DEFAULT_TIMER_SLACK;
	tunables->align_windows = true;

	spin_lock_init(&tunables->target_loads_lock);
	spin_lock_init(&tunables->above_hispeed_delay_lock);

	save_tunables(policy, tunables);
	return tunables;
}

static struct cpufreq_interactive_tunables *restore_tunables(
						struct cpufreq_policy *policy)
{
	int cpu;

	if (have_governor_per_policy())
		cpu = cpumask_first(policy->related_cpus);
	else
		cpu = 0;

	return per_cpu(cpuinfo, cpu).cached_tunables;
}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

static int cpufreq_governor_interactive(struct cpufreq_policy *policy,
		unsigned int event)
{
	int rc;
	unsigned int j;
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_interactive_tunables *tunables;
<<<<<<< HEAD
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };
	char speedchange_task_name[TASK_NAME_LEN];
=======
	unsigned long flags;
	int first_cpu;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (have_governor_per_policy())
		tunables = policy->governor_data;
	else
		tunables = common_tunables;

<<<<<<< HEAD
	WARN_ON(!tunables && (event != CPUFREQ_GOV_POLICY_INIT));
=======
	BUG_ON(!tunables && (event != CPUFREQ_GOV_POLICY_INIT));
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	switch (event) {
	case CPUFREQ_GOV_POLICY_INIT:
		if (have_governor_per_policy()) {
			WARN_ON(tunables);
		} else if (tunables) {
			tunables->usage_count++;
			policy->governor_data = tunables;
			return 0;
		}

<<<<<<< HEAD
		tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
		if (!tunables) {
			pr_err("%s: POLICY_INIT: kzalloc failed\n", __func__);
			return -ENOMEM;
		}

		if (!tuned_parameters[policy->cpu]) {
			tunables->above_hispeed_delay = default_above_hispeed_delay;
			tunables->nabove_hispeed_delay =
				ARRAY_SIZE(default_above_hispeed_delay);
			tunables->go_hispeed_load = DEFAULT_GO_HISPEED_LOAD;
			tunables->target_loads = default_target_loads;
			tunables->ntarget_loads = ARRAY_SIZE(default_target_loads);
			tunables->min_sample_time = DEFAULT_MIN_SAMPLE_TIME;
			tunables->timer_rate = DEFAULT_TIMER_RATE;
			tunables->boostpulse_duration_val = DEFAULT_MIN_SAMPLE_TIME;
			tunables->timer_slack_val = DEFAULT_TIMER_SLACK;
#ifdef CONFIG_MODE_AUTO_CHANGE
			tunables->multi_enter_time = DEFAULT_MULTI_ENTER_TIME;
			tunables->multi_enter_load = 4 * DEFAULT_TARGET_LOAD;
			tunables->multi_exit_time = DEFAULT_MULTI_EXIT_TIME;
			tunables->multi_exit_load = 4 * DEFAULT_TARGET_LOAD;
			tunables->single_enter_time = DEFAULT_SINGLE_ENTER_TIME;
			tunables->single_enter_load = DEFAULT_TARGET_LOAD;
			tunables->single_exit_time = DEFAULT_SINGLE_EXIT_TIME;
			tunables->single_exit_load = DEFAULT_TARGET_LOAD;

			cpufreq_param_set_init(tunables);
#endif
		} else {
			memcpy(tunables, tuned_parameters[policy->cpu], sizeof(*tunables));
			kfree(tuned_parameters[policy->cpu]);
		}
		tunables->usage_count = 1;

		/* update handle for get cpufreq_policy */
		tunables->policy = &policy->policy;

		spin_lock_init(&tunables->target_loads_lock);
		spin_lock_init(&tunables->above_hispeed_delay_lock);
#ifdef CONFIG_MODE_AUTO_CHANGE
		spin_lock_init(&tunables->mode_lock);
		spin_lock_init(&tunables->param_index_lock);
#endif

		policy->governor_data = tunables;
		if (!have_governor_per_policy())
			common_tunables = tunables;
=======
		first_cpu = cpumask_first(policy->related_cpus);
		for_each_cpu(j, policy->related_cpus)
			per_cpu(cpuinfo, j).first_cpu = first_cpu;

		tunables = restore_tunables(policy);
		if (!tunables) {
			tunables = alloc_tunable(policy);
			if (IS_ERR(tunables))
				return PTR_ERR(tunables);
		}

		tunables->usage_count = 1;
		policy->governor_data = tunables;
		if (!have_governor_per_policy()) {
			WARN_ON(cpufreq_get_global_kobject());
			common_tunables = tunables;
		}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

		rc = sysfs_create_group(get_governor_parent_kobj(policy),
				get_sysfs_attr());
		if (rc) {
			kfree(tunables);
			policy->governor_data = NULL;
<<<<<<< HEAD
			if (!have_governor_per_policy())
				common_tunables = NULL;
			return rc;
		}

		change_sysfs_owner(policy);

=======
			if (!have_governor_per_policy()) {
				common_tunables = NULL;
				cpufreq_put_global_kobject();
			}
			return rc;
		}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (!policy->governor->initialized) {
			idle_notifier_register(&cpufreq_interactive_idle_nb);
			cpufreq_register_notifier(&cpufreq_notifier_block,
					CPUFREQ_TRANSITION_NOTIFIER);
		}

<<<<<<< HEAD
=======
		if (tunables->use_sched_load)
			cpufreq_interactive_enable_sched_input(tunables);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		break;

	case CPUFREQ_GOV_POLICY_EXIT:
		if (!--tunables->usage_count) {
			if (policy->governor->initialized == 1) {
				cpufreq_unregister_notifier(&cpufreq_notifier_block,
						CPUFREQ_TRANSITION_NOTIFIER);
				idle_notifier_unregister(&cpufreq_interactive_idle_nb);
			}

			sysfs_remove_group(get_governor_parent_kobj(policy),
					get_sysfs_attr());
<<<<<<< HEAD

			tuned_parameters[policy->cpu] = kzalloc(sizeof(*tunables), GFP_KERNEL);
			if (!tuned_parameters[policy->cpu]) {
				pr_err("%s: POLICY_EXIT: kzalloc failed\n", __func__);
				return -ENOMEM;
			}
			memcpy(tuned_parameters[policy->cpu], tunables, sizeof(*tunables));
			kfree(tunables);
=======
			if (!have_governor_per_policy())
				cpufreq_put_global_kobject();
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			common_tunables = NULL;
		}

		policy->governor_data = NULL;
<<<<<<< HEAD
=======

		if (tunables->use_sched_load)
			cpufreq_interactive_disable_sched_input(tunables);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		break;

	case CPUFREQ_GOV_START:
		mutex_lock(&gov_lock);

		freq_table = cpufreq_frequency_get_table(policy->cpu);
		if (!tunables->hispeed_freq)
			tunables->hispeed_freq = policy->max;

<<<<<<< HEAD
#ifdef CONFIG_MODE_AUTO_CHANGE
		for (j = 0; j < MAX_PARAM_SET; j++) {
			if (!tunables->hispeed_freq_set[j])
				tunables->hispeed_freq_set[j] = policy->max;
		}
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			pcpu->policy = policy;
			pcpu->target_freq = policy->cur;
			pcpu->freq_table = freq_table;
			pcpu->floor_freq = pcpu->target_freq;
			pcpu->floor_validate_time =
				ktime_to_us(ktime_get());
<<<<<<< HEAD
			pcpu->hispeed_validate_time =
				pcpu->floor_validate_time;
			down_write(&pcpu->enable_sem);
			del_timer_sync(&pcpu->cpu_timer);
			del_timer_sync(&pcpu->cpu_slack_timer);
=======
			pcpu->local_fvtime = pcpu->floor_validate_time;
			pcpu->hispeed_validate_time =
				pcpu->floor_validate_time;
			pcpu->local_hvtime = pcpu->floor_validate_time;
			pcpu->max_freq = policy->max;
			down_write(&pcpu->enable_sem);
			del_timer_sync(&pcpu->cpu_timer);
			del_timer_sync(&pcpu->cpu_slack_timer);
			pcpu->last_evaluated_jiffy = get_jiffies_64();
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			cpufreq_interactive_timer_start(tunables, j);
			pcpu->governor_enabled = 1;
			up_write(&pcpu->enable_sem);
		}

<<<<<<< HEAD
		snprintf(speedchange_task_name, TASK_NAME_LEN, "cfinteractive%d\n",
					policy->cpu);

		tunables->speedchange_task =
			kthread_create(cpufreq_interactive_speedchange_task, NULL,
				       speedchange_task_name);
		if (IS_ERR(tunables->speedchange_task)) {
			mutex_unlock(&gov_lock);
			return PTR_ERR(tunables->speedchange_task);
		}

		sched_setscheduler_nocheck(tunables->speedchange_task, SCHED_FIFO, &param);
		get_task_struct(tunables->speedchange_task);

#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
		kthread_bind(tunables->speedchange_task, policy->cpu);
#endif

		/* NB: wake up so the thread does not look hung to the freezer */
		wake_up_process(tunables->speedchange_task);

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		mutex_unlock(&gov_lock);
		break;

	case CPUFREQ_GOV_STOP:
		mutex_lock(&gov_lock);
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			down_write(&pcpu->enable_sem);
			pcpu->governor_enabled = 0;
<<<<<<< HEAD
=======
			pcpu->target_freq = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			del_timer_sync(&pcpu->cpu_timer);
			del_timer_sync(&pcpu->cpu_slack_timer);
			up_write(&pcpu->enable_sem);
		}

<<<<<<< HEAD
		kthread_stop(tunables->speedchange_task);
		put_task_struct(tunables->speedchange_task);
		tunables->speedchange_task = NULL;

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		mutex_unlock(&gov_lock);
		break;

	case CPUFREQ_GOV_LIMITS:
		if (policy->max < policy->cur)
			__cpufreq_driver_target(policy,
					policy->max, CPUFREQ_RELATION_H);
		else if (policy->min > policy->cur)
			__cpufreq_driver_target(policy,
					policy->min, CPUFREQ_RELATION_L);
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);

<<<<<<< HEAD
			/* hold write semaphore to avoid race */
			down_write(&pcpu->enable_sem);
			if (pcpu->governor_enabled == 0) {
				up_write(&pcpu->enable_sem);
				continue;
			}

			/* update target_freq firstly */
=======
			down_read(&pcpu->enable_sem);
			if (pcpu->governor_enabled == 0) {
				up_read(&pcpu->enable_sem);
				continue;
			}

			spin_lock_irqsave(&pcpu->target_freq_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			if (policy->max < pcpu->target_freq)
				pcpu->target_freq = policy->max;
			else if (policy->min > pcpu->target_freq)
				pcpu->target_freq = policy->min;

<<<<<<< HEAD
			/* Reschedule timer.
=======
			spin_unlock_irqrestore(&pcpu->target_freq_lock, flags);
			up_read(&pcpu->enable_sem);

			/* Reschedule timer only if policy->max is raised.
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			 * Delete the timers, else the timer callback may
			 * return without re-arm the timer when failed
			 * acquire the semaphore. This race may cause timer
			 * stopped unexpectedly.
			 */
<<<<<<< HEAD
			del_timer_sync(&pcpu->cpu_timer);
			del_timer_sync(&pcpu->cpu_slack_timer);
			cpufreq_interactive_timer_start(tunables, j);
			up_write(&pcpu->enable_sem);
=======

			if (policy->max > pcpu->max_freq) {
				down_write(&pcpu->enable_sem);
				del_timer_sync(&pcpu->cpu_timer);
				del_timer_sync(&pcpu->cpu_slack_timer);
				cpufreq_interactive_timer_resched(j);
				up_write(&pcpu->enable_sem);
			}

			pcpu->max_freq = policy->max;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		}
		break;
	}
	return 0;
}

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_INTERACTIVE
static
#endif
struct cpufreq_governor cpufreq_gov_interactive = {
	.name = "interactive",
	.governor = cpufreq_governor_interactive,
	.max_transition_latency = 10000000,
	.owner = THIS_MODULE,
};

static void cpufreq_interactive_nop_timer(unsigned long data)
{
}

<<<<<<< HEAD
unsigned int cpufreq_interactive_get_hispeed_freq(int cpu)
{
	struct cpufreq_interactive_cpuinfo *pcpu =
			&per_cpu(cpuinfo, cpu);
	struct cpufreq_interactive_tunables *tunables;

	if (pcpu && pcpu->policy)
		tunables = pcpu->policy->governor_data;
	else
		return 0;

	if (!tunables)
		return 0;

	return tunables->hispeed_freq;
}

#ifdef CONFIG_ARCH_EXYNOS
static int cpufreq_interactive_cpu_min_qos_handler(struct notifier_block *b,
						unsigned long val, void *v)
{
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_interactive_tunables *tunables;
	unsigned long flags;
	int ret = NOTIFY_OK;
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
	int cpu = NR_CA7;
#else
	int cpu = 0;
#endif

	pcpu = &per_cpu(cpuinfo, cpu);

	mutex_lock(&gov_lock);
	down_read(&pcpu->enable_sem);
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		ret = NOTIFY_BAD;
		goto exit;
	}
	up_read(&pcpu->enable_sem);

	if (!pcpu->policy || !pcpu->policy->governor_data ||
		!pcpu->policy->user_policy.governor) {
		ret = NOTIFY_BAD;
		goto exit;
	}

	trace_cpufreq_interactive_cpu_min_qos(cpu, val, pcpu->policy->cur);

	if (val < pcpu->policy->cur) {
		tunables = pcpu->policy->governor_data;

		spin_lock_irqsave(&speedchange_cpumask_lock, flags);
		cpumask_set_cpu(cpu, &speedchange_cpumask);
		spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

		if (tunables->speedchange_task)
			wake_up_process(tunables->speedchange_task);
	}
exit:
	mutex_unlock(&gov_lock);
	return ret;
}

static struct notifier_block cpufreq_interactive_cpu_min_qos_notifier = {
	.notifier_call = cpufreq_interactive_cpu_min_qos_handler,
};

static int cpufreq_interactive_cpu_max_qos_handler(struct notifier_block *b,
						unsigned long val, void *v)
{
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_interactive_tunables *tunables;
	unsigned long flags;
	int ret = NOTIFY_OK;
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
	int cpu = NR_CA7;
#else
	int cpu = 0;
#endif

	pcpu = &per_cpu(cpuinfo, cpu);

	mutex_lock(&gov_lock);
	down_read(&pcpu->enable_sem);
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		ret =  NOTIFY_BAD;
		goto exit;
	}
	up_read(&pcpu->enable_sem);

	if (!pcpu->policy || !pcpu->policy->governor_data ||
		!pcpu->policy->user_policy.governor) {
		ret = NOTIFY_BAD;
		goto exit;
	}

	trace_cpufreq_interactive_cpu_max_qos(cpu, val, pcpu->policy->cur);

	if (val > pcpu->policy->cur) {
		tunables = pcpu->policy->governor_data;

		spin_lock_irqsave(&speedchange_cpumask_lock, flags);
		cpumask_set_cpu(cpu, &speedchange_cpumask);
		spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

		if (tunables->speedchange_task)
			wake_up_process(tunables->speedchange_task);
	}
exit:
	mutex_unlock(&gov_lock);
	return ret;
}

static struct notifier_block cpufreq_interactive_cpu_max_qos_notifier = {
	.notifier_call = cpufreq_interactive_cpu_max_qos_handler,
};

#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
static int cpufreq_interactive_kfc_min_qos_handler(struct notifier_block *b,
						unsigned long val, void *v)
{
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_interactive_tunables *tunables;
	unsigned long flags;
	int ret = NOTIFY_OK;

	pcpu = &per_cpu(cpuinfo, 0);

	mutex_lock(&gov_lock);
	down_read(&pcpu->enable_sem);
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		ret = NOTIFY_BAD;
		goto exit;
	}
	up_read(&pcpu->enable_sem);

	if (!pcpu->policy || !pcpu->policy->governor_data ||
		!pcpu->policy->user_policy.governor) {
		ret = NOTIFY_BAD;
		goto exit;
	}

	trace_cpufreq_interactive_kfc_min_qos(0, val, pcpu->policy->cur);

	if (val < pcpu->policy->cur) {
		tunables = pcpu->policy->governor_data;

		spin_lock_irqsave(&speedchange_cpumask_lock, flags);
		cpumask_set_cpu(0, &speedchange_cpumask);
		spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

		if (tunables->speedchange_task)
			wake_up_process(tunables->speedchange_task);
	}
exit:
	mutex_unlock(&gov_lock);
	return ret;
}

static struct notifier_block cpufreq_interactive_kfc_min_qos_notifier = {
	.notifier_call = cpufreq_interactive_kfc_min_qos_handler,
};

static int cpufreq_interactive_kfc_max_qos_handler(struct notifier_block *b,
						unsigned long val, void *v)
{
	struct cpufreq_interactive_cpuinfo *pcpu;
	struct cpufreq_interactive_tunables *tunables;
	unsigned long flags;
	int ret = NOTIFY_OK;

	pcpu = &per_cpu(cpuinfo, 0);

	mutex_lock(&gov_lock);
	down_read(&pcpu->enable_sem);
	if (!pcpu->governor_enabled) {
		up_read(&pcpu->enable_sem);
		ret = NOTIFY_BAD;
		goto exit;
	}
	up_read(&pcpu->enable_sem);

	if (!pcpu->policy ||!pcpu->policy->governor_data ||
		!pcpu->policy->user_policy.governor) {
		ret = NOTIFY_BAD;
		goto exit;
	}

	trace_cpufreq_interactive_kfc_max_qos(0, val, pcpu->policy->cur);

	if (val > pcpu->policy->cur) {
		tunables = pcpu->policy->governor_data;

		spin_lock_irqsave(&speedchange_cpumask_lock, flags);
		cpumask_set_cpu(0, &speedchange_cpumask);
		spin_unlock_irqrestore(&speedchange_cpumask_lock, flags);

		if (tunables->speedchange_task)
			wake_up_process(tunables->speedchange_task);
	}
exit:
	mutex_unlock(&gov_lock);
	return ret;
}

static struct notifier_block cpufreq_interactive_kfc_max_qos_notifier = {
	.notifier_call = cpufreq_interactive_kfc_max_qos_handler,
};
#endif
#endif

=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static int __init cpufreq_interactive_init(void)
{
	unsigned int i;
	struct cpufreq_interactive_cpuinfo *pcpu;
<<<<<<< HEAD
=======
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/* Initalize per-cpu timers */
	for_each_possible_cpu(i) {
		pcpu = &per_cpu(cpuinfo, i);
		init_timer_deferrable(&pcpu->cpu_timer);
		pcpu->cpu_timer.function = cpufreq_interactive_timer;
		pcpu->cpu_timer.data = i;
		init_timer(&pcpu->cpu_slack_timer);
		pcpu->cpu_slack_timer.function = cpufreq_interactive_nop_timer;
		spin_lock_init(&pcpu->load_lock);
<<<<<<< HEAD
=======
		spin_lock_init(&pcpu->target_freq_lock);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		init_rwsem(&pcpu->enable_sem);
	}

	spin_lock_init(&speedchange_cpumask_lock);
	mutex_init(&gov_lock);
<<<<<<< HEAD

#ifdef CONFIG_ARCH_EXYNOS
	pm_qos_add_notifier(PM_QOS_CPU_FREQ_MIN, &cpufreq_interactive_cpu_min_qos_notifier);
	pm_qos_add_notifier(PM_QOS_CPU_FREQ_MAX, &cpufreq_interactive_cpu_max_qos_notifier);
#ifdef CONFIG_ARM_EXYNOS_MP_CPUFREQ
	pm_qos_add_notifier(PM_QOS_KFC_FREQ_MIN, &cpufreq_interactive_kfc_min_qos_notifier);
	pm_qos_add_notifier(PM_QOS_KFC_FREQ_MAX, &cpufreq_interactive_kfc_max_qos_notifier);
#endif
#endif
=======
	mutex_init(&sched_lock);
	speedchange_task =
		kthread_create(cpufreq_interactive_speedchange_task, NULL,
			       "cfinteractive");
	if (IS_ERR(speedchange_task))
		return PTR_ERR(speedchange_task);

	sched_setscheduler_nocheck(speedchange_task, SCHED_FIFO, &param);
	get_task_struct(speedchange_task);

	/* NB: wake up so the thread does not look hung to the freezer */
	wake_up_process(speedchange_task);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	return cpufreq_register_governor(&cpufreq_gov_interactive);
}

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_INTERACTIVE
fs_initcall(cpufreq_interactive_init);
#else
module_init(cpufreq_interactive_init);
#endif

static void __exit cpufreq_interactive_exit(void)
{
<<<<<<< HEAD
	cpufreq_unregister_governor(&cpufreq_gov_interactive);
=======
	int cpu;
	struct cpufreq_interactive_cpuinfo *pcpu;

	cpufreq_unregister_governor(&cpufreq_gov_interactive);
	kthread_stop(speedchange_task);
	put_task_struct(speedchange_task);

	for_each_possible_cpu(cpu) {
		pcpu = &per_cpu(cpuinfo, cpu);
		kfree(pcpu->cached_tunables);
		pcpu->cached_tunables = NULL;
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
}

module_exit(cpufreq_interactive_exit);

MODULE_AUTHOR("Mike Chan <mike@android.com>");
MODULE_DESCRIPTION("'cpufreq_interactive' - A cpufreq governor for "
	"Latency sensitive workloads");
MODULE_LICENSE("GPL");
