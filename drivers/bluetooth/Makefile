#
# Makefile for the Linux Bluetooth HCI device drivers.
#

obj-$(CONFIG_BT_HCIVHCI)	+= hci_vhci.o
obj-$(CONFIG_BT_HCIUART)	+= hci_uart.o
obj-$(CONFIG_BT_HCIBCM203X)	+= bcm203x.o
obj-$(CONFIG_BT_HCIBPA10X)	+= bpa10x.o
obj-$(CONFIG_BT_HCIBFUSB)	+= bfusb.o
obj-$(CONFIG_BT_HCIDTL1)	+= dtl1_cs.o
obj-$(CONFIG_BT_HCIBT3C)	+= bt3c_cs.o
obj-$(CONFIG_BT_HCIBLUECARD)	+= bluecard_cs.o
obj-$(CONFIG_BT_HCIBTUART)	+= btuart_cs.o

obj-$(CONFIG_BT_HCIBTUSB)	+= btusb.o
obj-$(CONFIG_BT_HCIBTSDIO)	+= btsdio.o

obj-$(CONFIG_BT_ATH3K)		+= ath3k.o
obj-$(CONFIG_BT_MRVL)		+= btmrvl.o
obj-$(CONFIG_BT_MRVL_SDIO)	+= btmrvl_sdio.o
obj-$(CONFIG_BT_WILINK)		+= btwilink.o
<<<<<<< HEAD
=======
obj-$(CONFIG_MSM_BT_POWER)	+= bluetooth-power.o
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

btmrvl-y			:= btmrvl_main.o
btmrvl-$(CONFIG_DEBUG_FS)	+= btmrvl_debugfs.o

hci_uart-y				:= hci_ldisc.o
hci_uart-$(CONFIG_BT_HCIUART_H4)	+= hci_h4.o
hci_uart-$(CONFIG_BT_HCIUART_BCSP)	+= hci_bcsp.o
hci_uart-$(CONFIG_BT_HCIUART_LL)	+= hci_ll.o
hci_uart-$(CONFIG_BT_HCIUART_ATH3K)	+= hci_ath.o
hci_uart-$(CONFIG_BT_HCIUART_3WIRE)	+= hci_h5.o
<<<<<<< HEAD
hci_uart-objs				:= $(hci_uart-y)

obj-$(CONFIG_BT_BCM43457)		+= bcm43457.o
obj-$(CONFIG_BT_BCM4339)		+= bcm4339.o
obj-$(CONFIG_BT_BCM4354)		+= bcm4354.o
obj-$(CONFIG_BT_BCM4358)		+= bcm4358.o
obj-$(CONFIG_BT_BCM43455)		+= bcm43455.o
=======
hci_uart-$(CONFIG_BT_HCIUART_IBS)	+= hci_ibs.o
hci_uart-objs				:= $(hci_uart-y)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
