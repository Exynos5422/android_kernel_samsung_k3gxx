<<<<<<< HEAD
/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
=======
/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __GPIO_IR_RECV_H__
#define __GPIO_IR_RECV_H__

struct gpio_ir_recv_platform_data {
	int		gpio_nr;
	bool		active_low;
	u64		allowed_protos;
	const char	*map_name;
<<<<<<< HEAD
=======
	bool		can_wakeup;
	u32		swfi_latency;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
};

#endif /* __GPIO_IR_RECV_H__ */

