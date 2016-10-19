#
# Makefile for Kernel-based Virtual Machine module
#

plus_virt := $(call as-instr,.arch_extension virt,+virt)
ifeq ($(plus_virt),+virt)
	plus_virt_def := -DREQUIRES_VIRT=1
endif

ccflags-y += -Ivirt/kvm -Iarch/arm/kvm
CFLAGS_arm.o := -I. $(plus_virt_def)
CFLAGS_mmu.o := -I.

AFLAGS_init.o := -Wa,-march=armv7-a$(plus_virt)
AFLAGS_interrupts.o := -Wa,-march=armv7-a$(plus_virt)

<<<<<<< HEAD
kvm-arm-y = $(addprefix ../../../virt/kvm/, kvm_main.o coalesced_mmio.o)
=======
KVM := ../../../virt/kvm
kvm-arm-y = $(addprefix $(KVM)/, kvm_main.o coalesced_mmio.o)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

obj-y += kvm-arm.o init.o interrupts.o
obj-y += arm.o handle_exit.o guest.o mmu.o emulate.o reset.o
obj-y += coproc.o coproc_a15.o mmio.o psci.o perf.o
<<<<<<< HEAD
obj-$(CONFIG_KVM_ARM_VGIC) += vgic.o
obj-$(CONFIG_KVM_ARM_TIMER) += arch_timer.o
=======
obj-$(CONFIG_KVM_ARM_VGIC) += $(KVM)/arm/vgic.o
obj-$(CONFIG_KVM_ARM_TIMER) += $(KVM)/arm/arch_timer.o
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
