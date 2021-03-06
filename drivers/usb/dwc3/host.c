/**
 * host.c - DesignWare USB3 DRD Controller Host Glue
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *
<<<<<<< HEAD
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
=======
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2, as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
 */

#include <linux/platform_device.h>

#include "core.h"
<<<<<<< HEAD
=======
#include "xhci.h"
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

int dwc3_host_init(struct dwc3 *dwc)
{
	struct platform_device	*xhci;
	int			ret;
<<<<<<< HEAD
=======
	struct xhci_plat_data	pdata;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
	if (!xhci) {
		dev_err(dwc->dev, "couldn't allocate xHCI device\n");
		ret = -ENOMEM;
		goto err0;
	}

	dma_set_coherent_mask(&xhci->dev, dwc->dev->coherent_dma_mask);

	xhci->dev.parent	= dwc->dev;
	xhci->dev.dma_mask	= dwc->dev->dma_mask;
	xhci->dev.dma_parms	= dwc->dev->dma_parms;

	dwc->xhci = xhci;
<<<<<<< HEAD
=======
	pdata.vendor = ((dwc->revision & DWC3_GSNPSID_MASK) >>
			__ffs(DWC3_GSNPSID_MASK) & DWC3_GSNPSREV_MASK);
	pdata.revision = dwc->revision & DWC3_GSNPSREV_MASK;

	ret = platform_device_add_data(xhci, (const void *) &pdata,
			sizeof(struct xhci_plat_data));
	if (ret) {
		dev_err(dwc->dev, "couldn't add pdata to xHCI device\n");
		goto err1;
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	ret = platform_device_add_resources(xhci, dwc->xhci_resources,
						DWC3_XHCI_RESOURCES_NUM);
	if (ret) {
		dev_err(dwc->dev, "couldn't add resources to xHCI device\n");
		goto err1;
	}

<<<<<<< HEAD
	/* If OTG is available, it will take care of this */
=======
	/* Add XHCI device if !OTG, otherwise OTG takes care of this */
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (!dwc->dotg) {
		ret = platform_device_add(xhci);
		if (ret) {
			dev_err(dwc->dev, "failed to register xHCI device\n");
			goto err1;
		}
	}

	return 0;

err1:
	platform_device_put(xhci);

err0:
	return ret;
}

void dwc3_host_exit(struct dwc3 *dwc)
{
	if (!dwc->dotg)
		platform_device_unregister(dwc->xhci);
}
