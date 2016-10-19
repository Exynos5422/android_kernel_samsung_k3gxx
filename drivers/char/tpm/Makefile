#
# Makefile for the kernel tpm device drivers.
#
<<<<<<< HEAD
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
obj-$(CONFIG_TCG_TPM) += tpm.o
ifdef CONFIG_ACPI
	obj-$(CONFIG_TCG_TPM) += tpm_bios.o
	tpm_bios-objs += tpm_eventlog.o tpm_acpi.o tpm_ppi.o
else
ifdef CONFIG_TCG_IBMVTPM
	obj-$(CONFIG_TCG_TPM) += tpm_bios.o
	tpm_bios-objs += tpm_eventlog.o tpm_of.o
endif
endif
obj-$(CONFIG_TCG_TIS) += tpm_tis.o
obj-$(CONFIG_TCG_TIS_I2C_INFINEON) += tpm_i2c_infineon.o
obj-$(CONFIG_TCG_NSC) += tpm_nsc.o
obj-$(CONFIG_TCG_ATMEL) += tpm_atmel.o
obj-$(CONFIG_TCG_INFINEON) += tpm_infineon.o
obj-$(CONFIG_TCG_IBMVTPM) += tpm_ibmvtpm.o
obj-$(CONFIG_TCG_ST33_I2C) += tpm_i2c_stm_st33.o
