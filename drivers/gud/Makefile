<<<<<<< HEAD
obj-$(CONFIG_SOC_EXYNOS5422) += gud-exynos5422/
ifeq ($(CONFIG_SOC_EXYNOS5430_TBASE302), y)
	obj-$(CONFIG_SOC_EXYNOS5430_TBASE302) += gud-exynos5430_tbase302/
else ifeq ($(CONFIG_SOC_EXYNOS5430), y)
	obj-$(CONFIG_SOC_EXYNOS5430) += gud-exynos5430/
endif
ifeq ($(CONFIG_SOC_EXYNOS5433_TBASE302), y)
	obj-$(CONFIG_SOC_EXYNOS5433_TBASE302) += gud-exynos5433_tbase302/
else ifeq ($(CONFIG_SOC_EXYNOS5433), y)
	obj-$(CONFIG_SOC_EXYNOS5433) += gud-exynos5433/
endif
obj-$(CONFIG_TRUSTONIC_TRUSTED_UI) += TlcTui/
=======
#
# Makefile for the kernel mobicore drivers
#
GUD_ROOT_FOLDER := drivers/gud
# add our modules to kernel.
obj-$(CONFIG_MOBICORE_API) += mcKernelApi.o
obj-$(CONFIG_MOBICORE_SUPPORT) += mcDrvModule.o

mcDrvModule-objs := MobiCoreDriver/logging.o \
		MobiCoreDriver/ops.o \
		MobiCoreDriver/mem.o \
		MobiCoreDriver/api.o \
		MobiCoreDriver/pm.o \
		MobiCoreDriver/main.o

mcKernelApi-objs := MobiCoreKernelApi/main.o \
		MobiCoreKernelApi/clientlib.o \
		MobiCoreKernelApi/device.o \
		MobiCoreKernelApi/session.o \
		MobiCoreKernelApi/connection.o

# Release mode by default
ccflags-y := -DNDEBUG -I$(GUD_ROOT_FOLDER)
ccflags-y += -Wno-declaration-after-statement

#Netlink changed arguments number
ccflags-y += -DMC_NETLINK_COMPAT_V37

ccflags-$(CONFIG_MOBICORE_DEBUG) += -DDEBUG
ccflags-$(CONFIG_MOBICORE_VERBOSE) += -DDEBUG_VERBOSE

# Choose one platform from the folder
MOBICORE_PLATFORM := $(shell (ls -1 $(srctree)/$(GUD_ROOT_FOLDER)/MobiCoreDriver/platforms | tail -1) )
# Use the available platform folder
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/platforms/$(MOBICORE_PLATFORM)
# MobiCore Driver includes
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/public
# MobiCore KernelApi required incldes
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/include
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/public
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
