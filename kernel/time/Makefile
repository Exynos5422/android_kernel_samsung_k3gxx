obj-y += timekeeping.o ntp.o clocksource.o jiffies.o timer_list.o
obj-y += timeconv.o posix-clock.o alarmtimer.o

obj-$(CONFIG_GENERIC_CLOCKEVENTS_BUILD)		+= clockevents.o
obj-$(CONFIG_GENERIC_CLOCKEVENTS)		+= tick-common.o
<<<<<<< HEAD
obj-$(CONFIG_GENERIC_CLOCKEVENTS_BROADCAST)	+= tick-broadcast.o
=======
obj-$(CONFIG_GENERIC_CLOCKEVENTS_BROADCAST)	+= tick-broadcast.o tick-broadcast-hrtimer.o
obj-$(CONFIG_GENERIC_SCHED_CLOCK)		+= sched_clock.o
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
obj-$(CONFIG_TICK_ONESHOT)			+= tick-oneshot.o
obj-$(CONFIG_TICK_ONESHOT)			+= tick-sched.o
obj-$(CONFIG_TIMER_STATS)			+= timer_stats.o
