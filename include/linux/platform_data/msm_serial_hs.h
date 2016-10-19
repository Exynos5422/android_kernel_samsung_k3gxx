/*
 * Copyright (C) 2008 Google, Inc.
<<<<<<< HEAD
=======
 * Copyright (C) 2010-2014, The Linux Foundation. All rights reserved.
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
 * Author: Nick Pelly <npelly@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_MSM_SERIAL_HS_H
#define __ASM_ARCH_MSM_SERIAL_HS_H

#include <linux/serial_core.h>

<<<<<<< HEAD
/* API to request the uart clock off or on for low power management
 * Clients should call request_clock_off() when no uart data is expected,
 * and must call request_clock_on() before any further uart data can be
 * received. */
extern void msm_hs_request_clock_off(struct uart_port *uport);
extern void msm_hs_request_clock_on(struct uart_port *uport);

/**
 * struct msm_serial_hs_platform_data
 * @rx_wakeup_irq: Rx activity irq
 * @rx_to_inject: extra character to be inserted to Rx tty on wakeup
 * @inject_rx: 1 = insert rx_to_inject. 0 = do not insert extra character
 * @exit_lpm_cb: function called before every Tx transaction
 *
 * This is an optional structure required for UART Rx GPIO IRQ based
 * wakeup from low power state. UART wakeup can be triggered by RX activity
 * (using a wakeup GPIO on the UART RX pin). This should only be used if
 * there is not a wakeup GPIO on the UART CTS, and the first RX byte is
 * known (eg., with the Bluetooth Texas Instruments HCILL protocol),
 * since the first RX byte will always be lost. RTS will be asserted even
 * while the UART is clocked off in this mode of operation.
 */
struct msm_serial_hs_platform_data {
	int rx_wakeup_irq;
	unsigned char inject_rx_on_wakeup;
	char rx_to_inject;
	void (*exit_lpm_cb)(struct uart_port *);
};

=======
/**
 * struct msm_serial_hs_platform_data - platform device data
 *					for msm hsuart device
 * @wakeup_irq : IRQ line to be configured as Wakeup source.
 * @inject_rx_on_wakeup : Set 1 if specific character to be inserted on wakeup
 * @rx_to_inject : Character to be inserted on wakeup
 * @gpio_config : Configure gpios that are used for uart communication
 * @userid : User-defined number to be used to enumerate device as tty<userid>
 * @uart_tx_gpio: GPIO number for UART Tx Line.
 * @uart_rx_gpio: GPIO number for UART Rx Line.
 * @uart_cts_gpio: GPIO number for UART CTS Line.
 * @uart_rfr_gpio: GPIO number for UART RFR Line.
 * @bam_tx_ep_pipe_index : BAM TX Endpoint Pipe Index for HSUART
 * @bam_tx_ep_pipe_index : BAM RX Endpoint Pipe Index for HSUART
 * @no_suspend_delay : Flag used to make system go to suspend
 * immediately or not
 * @obs: Flag to enable out of band sleep feature support
 */
struct msm_serial_hs_platform_data {
	int wakeup_irq;  /* wakeup irq */
	bool inject_rx_on_wakeup;
	u8 rx_to_inject;
	int (*gpio_config)(int);
	int userid;

	int uart_tx_gpio;
	int uart_rx_gpio;
	int uart_cts_gpio;
	int uart_rfr_gpio;
	unsigned bam_tx_ep_pipe_index;
	unsigned bam_rx_ep_pipe_index;
	bool no_suspend_delay;
	bool obs;
};

/* return true when tx is empty */
unsigned int msm_hs_tx_empty(struct uart_port *uport);
void msm_hs_request_clock_off(struct uart_port *uport);
void msm_hs_request_clock_on(struct uart_port *uport);
struct uart_port *msm_hs_get_uart_port(int port_index);
void msm_hs_set_mctrl(struct uart_port *uport,
				    unsigned int mctrl);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#endif
