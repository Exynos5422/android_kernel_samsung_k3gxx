obj-$(CONFIG_WIL6210) += wil6210.o

<<<<<<< HEAD
wil6210-objs := main.o
wil6210-objs += netdev.o
wil6210-objs += cfg80211.o
wil6210-objs += pcie_bus.o
wil6210-objs += debugfs.o
wil6210-objs += wmi.o
wil6210-objs += interrupt.o
wil6210-objs += txrx.o

ifeq (, $(findstring -W,$(EXTRA_CFLAGS)))
	subdir-ccflags-y += -Werror
endif
=======
wil6210-y := main.o
wil6210-y += netdev.o
wil6210-y += cfg80211.o
wil6210-y += pcie_bus.o
wil6210-y += debugfs.o
wil6210-y += wmi.o
wil6210-y += interrupt.o
wil6210-y += txrx.o
wil6210-y += debug.o
wil6210-y += rx_reorder.o
wil6210-y += fw.o
wil6210-$(CONFIG_WIL6210_TRACING) += trace.o
wil6210-y += wil_platform.o
wil6210-$(CONFIG_WIL6210_PLATFORM_MSM) += wil_platform_msm.o

# for tracing framework to find trace.h
CFLAGS_trace.o := -I$(src)

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
subdir-ccflags-y += -D__CHECK_ENDIAN__
