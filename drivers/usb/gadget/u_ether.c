/*
 * u_ether.c -- Ethernet-over-USB link layer utilities for Gadget stack
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/ctype.h>
<<<<<<< HEAD
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>

=======
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/msm_rmnet.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/notifier.h>
#include <linux/cpufreq.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include "u_ether.h"


/*
 * This component encapsulates the Ethernet link glue needed to provide
 * one (!) network link through the USB gadget stack, normally "usb0".
 *
 * The control and data models are handled by the function driver which
 * connects to this code; such as CDC Ethernet (ECM or EEM),
 * "CDC Subset", or RNDIS.  That includes all descriptor and endpoint
 * management.
 *
 * Link level addressing is handled by this component using module
 * parameters; if no such parameters are provided, random link level
 * addresses are used.  Each end of the link uses one address.  The
 * host end address is exported in various ways, and is often recorded
 * in configuration databases.
 *
 * The driver which assembles each configuration using such a link is
 * responsible for ensuring that each configuration includes at most one
 * instance of is network link.  (The network layer provides ways for
 * this single "physical" link to be used by multiple virtual links.)
 */

#define UETH__VERSION	"29-May-2008"

<<<<<<< HEAD
struct eth_dev {
	/* lock is held while accessing port_usb
	 * or updating its backlink port_usb->ioport
=======
static struct workqueue_struct	*uether_wq;
static struct workqueue_struct	*uether_tx_wq;

static int tx_start_threshold = 1500;
module_param(tx_start_threshold, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_start_threshold,
	"Threashold to start stopped network queue");

static int tx_stop_threshold = 2000;
module_param(tx_stop_threshold, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_stop_threshold,
	"Threashold to stop network queue");

static unsigned int min_cpu_freq;
module_param(min_cpu_freq, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(min_cpu_freq,
	"to set minimum cpu frquency to when ethernet ifc is active");

/* this refers to max number sgs per transfer
 * which includes headers/data packets
 */
#define DL_MAX_PKTS_PER_XFER	20

enum ifc_state {
	ETH_UNDEFINED,
	ETH_STOP,
	ETH_START,
};

struct eth_dev {
	/* lock is held while accessing port_usb
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	 */
	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock;	/* guard {rx,tx}_reqs */
	struct list_head	tx_reqs, rx_reqs;
<<<<<<< HEAD
	atomic_t		tx_qlen;
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
=======
	unsigned		tx_qlen;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/* Minimum number of TX USB request queued to UDC */
#define TX_REQ_THRESHOLD	5
	int			no_tx_req_used;
	int			tx_skb_hold_count;
	u32			tx_req_bufsize;
<<<<<<< HEAD
#endif
=======
	struct sk_buff_head	tx_skb_q;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	struct sk_buff_head	rx_frames;

	unsigned		header_len;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	unsigned int		ul_max_pkts_per_xfer;
	unsigned int		dl_max_pkts_per_xfer;
#endif
=======
	unsigned int		ul_max_pkts_per_xfer;
	unsigned int		dl_max_pkts_per_xfer;
	uint32_t		dl_max_xfer_size;
	bool			rx_trigger_enabled;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	struct sk_buff		*(*wrap)(struct gether *, struct sk_buff *skb);
	int			(*unwrap)(struct gether *,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	struct work_struct	work;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	struct tasklet_struct rx_tsk;
#endif

	unsigned long		todo;
=======
	struct work_struct	rx_work;
	struct work_struct	tx_work;

	unsigned long		todo;
	unsigned long		flags;
	unsigned short		rx_needed_headroom;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#define	WORK_RX_MEMORY		0

	bool			zlp;
	u8			host_mac[ETH_ALEN];
<<<<<<< HEAD
};

/*-------------------------------------------------------------------------*/

=======

	/* stats */
	unsigned long		tx_throttle;
	unsigned long		rx_throttle;
	unsigned int		tx_aggr_cnt[DL_MAX_PKTS_PER_XFER];
	unsigned int		tx_pkts_rcvd;
	unsigned int		loop_brk_cnt;
	struct dentry		*uether_dent;
	struct dentry		*uether_dfile;

	enum ifc_state		state;
	struct notifier_block	cpufreq_notifier;
	struct work_struct	cpu_policy_w;
};

/* when sg is enabled, sg_ctx is used to track skb each usb request will
 * xfer
 */
struct sg_ctx {
	struct sk_buff_head	skbs;
};


static void uether_debugfs_init(struct eth_dev *dev, const char *n);
static void uether_debugfs_exit(struct eth_dev *dev);

/*-------------------------------------------------------------------------*/


>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#define RX_EXTRA	20	/* bytes guarding against rx overflows */

#define DEFAULT_QLEN	2	/* double buffering by default */

<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
static unsigned qmult = 10;
#elif defined(CONFIG_USB_GADGET_DUALSPEED)
static unsigned qmult = 5;
#else	/* full speed (low speed doesn't do bulk) */
#define qmult		1
#endif
#if defined(CONFIG_USB_RNDIS_MULTIPACKET)||defined(CONFIG_USB_GADGET_DUALSPEED)
module_param(qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult, "queue length multiplier at high/super speed");
#endif
=======
static unsigned qmult = 20;
module_param(qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult, "queue length multiplier at high/super speed");

/*
 * Usually downlink rates are higher than uplink rates and it
 * deserve higher number of requests. For CAT-6 data rates of
 * 300Mbps (~30 packets per milli-sec) 40 usb request may not
 * be sufficient. At this rate and with interrupt moderation
 * of interconnect, data can be very bursty. tx_qmult is the
 * additional multipler on qmult.
 */
static unsigned tx_qmult = 1;
module_param(tx_qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(tx_qmult, "Additional queue length multiplier for tx");
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

/* for dual-speed hardware, use deeper queues at high/super speed */
static inline int qlen(struct usb_gadget *gadget)
{
	if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
					    gadget->speed == USB_SPEED_SUPER))
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
}

/*-------------------------------------------------------------------------*/
<<<<<<< HEAD
=======
#define U_ETHER_RX_PENDING_TSHOLD 500

static unsigned int u_ether_rx_pending_thld = U_ETHER_RX_PENDING_TSHOLD;
module_param(u_ether_rx_pending_thld, uint, S_IRUGO | S_IWUSR);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

/* REVISIT there must be a better way than having two sets
 * of debug calls ...
 */

#undef DBG
#undef VDBG
#undef ERROR
#undef INFO

#define xprintk(d, level, fmt, args...) \
	printk(level "%s: " fmt , (d)->net->name , ## args)

#ifdef DEBUG
#undef DEBUG
#define DBG(dev, fmt, args...) \
	xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE_DEBUG
#define VDBG	DBG
#else
#define VDBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#define ERROR(dev, fmt, args...) \
	xprintk(dev , KERN_ERR , fmt , ## args)
#define INFO(dev, fmt, args...) \
	xprintk(dev , KERN_INFO , fmt , ## args)

/*-------------------------------------------------------------------------*/

/* NETWORK DRIVER HOOKUP (to the layer above this driver) */

static int ueth_change_mtu(struct net_device *net, int new_mtu)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
	int		status = 0;

	/* don't change MTU on "live" link (peer won't know) */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		status = -EBUSY;
	else if (new_mtu <= ETH_HLEN || new_mtu > ETH_FRAME_LEN)
		status = -ERANGE;
	else
		net->mtu = new_mtu;
	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

<<<<<<< HEAD
=======
static int ueth_change_mtu_ip(struct net_device *net, int new_mtu)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
	int		status = 0;

	spin_lock_irqsave(&dev->lock, flags);
	if (new_mtu <= 0)
		status = -EINVAL;
	else
		net->mtu = new_mtu;

	DBG(dev, "[%s] MTU change: old=%d new=%d\n", net->name,
					net->mtu, new_mtu);
	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static void eth_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *p)
{
	struct eth_dev *dev = netdev_priv(net);

	strlcpy(p->driver, "g_ether", sizeof(p->driver));
	strlcpy(p->version, UETH__VERSION, sizeof(p->version));
	strlcpy(p->fw_version, dev->gadget->name, sizeof(p->fw_version));
	strlcpy(p->bus_info, dev_name(&dev->gadget->dev), sizeof(p->bus_info));
}

/* REVISIT can also support:
 *   - WOL (by tracking suspends and issuing remote wakeup)
 *   - msglevel (implies updated messaging)
 *   - ... probably more ethtool ops
 */

static const struct ethtool_ops ops = {
	.get_drvinfo = eth_get_drvinfo,
	.get_link = ethtool_op_get_link,
};

static void defer_kevent(struct eth_dev *dev, int flag)
{
	if (test_and_set_bit(flag, &dev->todo))
		return;
	if (!schedule_work(&dev->work))
		ERROR(dev, "kevent %d may have been dropped\n", flag);
	else
		DBG(dev, "kevent %d scheduled\n", flag);
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req);
<<<<<<< HEAD

#ifdef CONFIG_USB_RNDIS_MULTIPACKET
static inline void schedule_uether_rx(struct eth_dev *dev)
{
	tasklet_schedule(&dev->rx_tsk);
}
#endif
=======
static void tx_complete(struct usb_ep *ep, struct usb_request *req);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static int
rx_submit(struct eth_dev *dev, struct usb_request *req, gfp_t gfp_flags)
{
	struct sk_buff	*skb;
	int		retval = -ENOMEM;
	size_t		size = 0;
	struct usb_ep	*out;
	unsigned long	flags;
<<<<<<< HEAD
=======
	unsigned short reserve_headroom = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		out = dev->port_usb->out_ep;
	else
		out = NULL;
<<<<<<< HEAD
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!out)
		return -ENOTCONN;
=======

	if (!out) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return -ENOTCONN;
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83


	/* Padding up to RX_EXTRA handles minor disagreements with host.
	 * Normally we use the USB "terminate on short read" convention;
	 * so allow up to (N*maxpacket), since that memory is normally
	 * already allocated.  Some hardware doesn't deal well with short
	 * reads (e.g. DMA must be N*maxpacket), so for now don't trim a
	 * byte off the end (to force hardware errors on overflow).
	 *
	 * RNDIS uses internal framing, and explicitly allows senders to
	 * pad to end-of-packet.  That's potentially nice for speed, but
	 * means receivers can't recover lost synch on their own (because
	 * new packets don't only start after a short RX).
	 */
	size += sizeof(struct ethhdr) + dev->net->mtu + RX_EXTRA;
	size += dev->port_usb->header_len;
	size += out->maxpacket - 1;
	size -= size % out->maxpacket;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	if (dev->ul_max_pkts_per_xfer)
		size *= dev->ul_max_pkts_per_xfer;
#endif
	if (dev->port_usb->is_fixed)
		size = max_t(size_t, size, dev->port_usb->fixed_out_len);

	pr_debug("%s: size: %d", __func__, size);
	skb = alloc_skb(size + NET_IP_ALIGN, gfp_flags);
=======

	if (dev->ul_max_pkts_per_xfer)
		size *= dev->ul_max_pkts_per_xfer;

	if (dev->port_usb->is_fixed)
		size = max_t(size_t, size, dev->port_usb->fixed_out_len);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (dev->rx_needed_headroom)
		reserve_headroom = ALIGN(dev->rx_needed_headroom, 4);

	pr_debug("%s: size: %zu + %d(hr)", __func__, size, reserve_headroom);

	skb = alloc_skb(size + reserve_headroom, gfp_flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (skb == NULL) {
		DBG(dev, "no rx skb\n");
		goto enomem;
	}

	/* Some platforms perform better when IP packets are aligned,
	 * but on at least one, checksumming fails otherwise.  Note:
	 * RNDIS headers involve variable numbers of LE32 values.
	 */
<<<<<<< HEAD
	skb_reserve(skb, NET_IP_ALIGN);

	req->buf = skb->data;
	req->length = size;
	req->complete = rx_complete;
=======
	skb_reserve(skb, reserve_headroom);

	req->buf = skb->data;
	req->length = size;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	req->context = skb;

	retval = usb_ep_queue(out, req, gfp_flags);
	if (retval == -ENOMEM)
enomem:
		defer_kevent(dev, WORK_RX_MEMORY);
	if (retval) {
		DBG(dev, "rx submit --> %d\n", retval);
		if (skb)
			dev_kfree_skb_any(skb);
<<<<<<< HEAD
#ifndef CONFIG_USB_RNDIS_MULTIPACKET
		spin_lock_irqsave(&dev->req_lock, flags);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}
	return retval;
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	struct sk_buff	*skb = req->context;
	bool		queue = 0;
#else
	struct sk_buff	*skb = req->context, *skb2;
#endif
	struct eth_dev	*dev = ep->driver_data;
	int		status = req->status;
=======
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;
	int		status = req->status;
	bool		queue = 0;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	switch (status) {

	/* normal completion */
	case 0:
		skb_put(skb, req->actual);

		if (dev->unwrap) {
			unsigned long	flags;

			spin_lock_irqsave(&dev->lock, flags);
			if (dev->port_usb) {
				status = dev->unwrap(dev->port_usb,
							skb,
							&dev->rx_frames);
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
				if (status == -EINVAL)
					dev->net->stats.rx_errors++;
				else if (status == -EOVERFLOW)
					dev->net->stats.rx_over_errors++;
<<<<<<< HEAD
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			} else {
				dev_kfree_skb_any(skb);
				status = -ENOTCONN;
			}
			spin_unlock_irqrestore(&dev->lock, flags);
		} else {
			skb_queue_tail(&dev->rx_frames, skb);
		}
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET

		if (!status)
			queue = 1;
#else
		skb = NULL;

		skb2 = skb_dequeue(&dev->rx_frames);
		while (skb2) {
			if (status < 0
					|| ETH_HLEN > skb2->len
					|| skb2->len > VLAN_ETH_FRAME_LEN) {
#ifdef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
				/*
					Need to revisit net->mtu  does not include header size incase of changed MTU
				*/
				if(!strcmp(dev->port_usb->func.name,"ncm")) {
					if (status < 0
						|| ETH_HLEN > skb2->len
						|| skb2->len > (dev->net->mtu + ETH_HLEN)) {
						printk(KERN_DEBUG "usb: %s  drop incase of NCM rx length %d\n",__func__,skb2->len);
					} else {
						printk(KERN_DEBUG "usb: %s  Dont drop incase of NCM rx length %d\n",__func__,skb2->len);
						goto process_frame;
					}
				}
				printk(KERN_DEBUG "usb: %s Drop rx length %d\n",__func__,skb2->len);
#endif
				dev->net->stats.rx_errors++;
				dev->net->stats.rx_length_errors++;
				DBG(dev, "rx length %d\n", skb2->len);
				dev_kfree_skb_any(skb2);
				goto next_frame;
			}
#ifdef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
process_frame:
#endif
			skb2->protocol = eth_type_trans(skb2, dev->net);
			dev->net->stats.rx_packets++;
			dev->net->stats.rx_bytes += skb2->len;

			/* no buffer copies needed, unless hardware can't
			 * use skb buffers.
			 */
			status = netif_rx(skb2);
next_frame:
			skb2 = skb_dequeue(&dev->rx_frames);
		}
#endif
=======

		if (!status)
			queue = 1;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		VDBG(dev, "rx shutdown, code %d\n", status);
		goto quiesce;

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		DBG(dev, "rx %s reset\n", ep->name);
		defer_kevent(dev, WORK_RX_MEMORY);
quiesce:
		dev_kfree_skb_any(skb);
		goto clean;

	/* data overrun */
	case -EOVERFLOW:
		dev->net->stats.rx_over_errors++;
		/* FALLTHROUGH */

	default:
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		queue = 1;
		dev_kfree_skb_any(skb);
#endif
=======
		queue = 1;
		dev_kfree_skb_any(skb);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		dev->net->stats.rx_errors++;
		DBG(dev, "rx status %d\n", status);
		break;
	}
<<<<<<< HEAD
#ifndef CONFIG_USB_RNDIS_MULTIPACKET
	if (skb)
		dev_kfree_skb_any(skb);
	if (!netif_running(dev->net)) {
#endif
clean:
	spin_lock(&dev->req_lock);
	list_add(&req->list, &dev->rx_reqs);
	spin_unlock(&dev->req_lock);
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	if (queue)
		schedule_uether_rx(dev);
#else
		req = NULL;
	}
	if (req)
		rx_submit(dev, req, GFP_ATOMIC);
#endif
}

static int prealloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
	unsigned		i;
	struct usb_request	*req;
=======

clean:
	if (queue && dev->rx_frames.qlen <= u_ether_rx_pending_thld) {
		if (rx_submit(dev, req, GFP_ATOMIC) < 0) {
			spin_lock(&dev->req_lock);
			list_add(&req->list, &dev->rx_reqs);
			spin_unlock(&dev->req_lock);
		}
	} else {
		/* rx buffers draining is delayed,defer further queuing to wq */
		if (queue)
			dev->rx_throttle++;
		spin_lock(&dev->req_lock);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock(&dev->req_lock);
	}

	if (queue)
		queue_work(uether_wq, &dev->rx_work);
}

static int prealloc(struct list_head *list,
		struct usb_ep *ep, unsigned n,
		bool sg_supported, int hlen)
{
	unsigned		i;
	struct usb_request	*req;
	bool			usb_in;
	struct sg_ctx		*sg_ctx;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (!n)
		return -ENOMEM;

	/* queue/recycle up to N requests */
	i = n;
	list_for_each_entry(req, list, list) {
		if (i-- == 0)
			goto extra;
	}
<<<<<<< HEAD
=======

	if (ep->desc->bEndpointAddress & USB_DIR_IN)
		usb_in = true;
	else
		usb_in = false;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	while (i--) {
		req = usb_ep_alloc_request(ep, GFP_ATOMIC);
		if (!req)
			return list_empty(list) ? -ENOMEM : 0;
<<<<<<< HEAD
=======
		/* update completion handler */
		if (usb_in) {
			req->complete = tx_complete;
			if (!sg_supported)
				goto add_list;
			req->sg = kmalloc(
					DL_MAX_PKTS_PER_XFER *
						sizeof(struct scatterlist),
					GFP_ATOMIC);
			if (!req->sg)
				goto extra;
			sg_ctx = kmalloc(sizeof(*sg_ctx), GFP_ATOMIC);
			if (!sg_ctx)
				goto extra;
			req->context = sg_ctx;
			req->buf = kzalloc(DL_MAX_PKTS_PER_XFER * hlen,
						GFP_ATOMIC);
		} else {
			req->complete = rx_complete;
		}
add_list:
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		list_add(&req->list, list);
	}
	return 0;

extra:
	/* free extras */
	for (;;) {
		struct list_head	*next;

		next = req->list.next;
		list_del(&req->list);
<<<<<<< HEAD
=======

		if (sg_supported) {
			kfree(req->sg);
			kfree(req->context);
			kfree(req->buf);
		}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		usb_ep_free_request(ep, req);

		if (next == list)
			break;

		req = container_of(next, struct usb_request, list);
	}
	return 0;
}

static int alloc_requests(struct eth_dev *dev, struct gether *link, unsigned n)
{
	int	status;

	spin_lock(&dev->req_lock);
<<<<<<< HEAD
	status = prealloc(&dev->tx_reqs, link->in_ep, n);
	if (status < 0)
		goto fail;
	status = prealloc(&dev->rx_reqs, link->out_ep, n);
=======
	status = prealloc(&dev->tx_reqs, link->in_ep, n * tx_qmult,
				dev->gadget->sg_supported,
				dev->header_len);
	if (status < 0)
		goto fail;
	status = prealloc(&dev->rx_reqs, link->out_ep, n,
				dev->gadget->sg_supported,
				dev->header_len);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (status < 0)
		goto fail;
	goto done;
fail:
	DBG(dev, "can't alloc requests\n");
done:
	spin_unlock(&dev->req_lock);
	return status;
}

static void rx_fill(struct eth_dev *dev, gfp_t gfp_flags)
{
	struct usb_request	*req;
	unsigned long		flags;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	int			req_cnt = 0;
#endif
=======
	int			req_cnt = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/* fill unused rxq slots with some skb */
	spin_lock_irqsave(&dev->req_lock, flags);
	while (!list_empty(&dev->rx_reqs)) {
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		/* break the nexus of continuous completion and re-submission*/
		if (++req_cnt > qlen(dev->gadget))
			break;
#endif
=======
		/* break the nexus of continuous completion and re-submission*/
		if (++req_cnt > qlen(dev->gadget))
			break;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del_init(&req->list);
		spin_unlock_irqrestore(&dev->req_lock, flags);

		if (rx_submit(dev, req, gfp_flags) < 0) {
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
			spin_lock_irqsave(&dev->req_lock, flags);
			list_add(&req->list, &dev->rx_reqs);
			spin_unlock_irqrestore(&dev->req_lock, flags);
#endif
=======
			spin_lock_irqsave(&dev->req_lock, flags);
			list_add(&req->list, &dev->rx_reqs);
			spin_unlock_irqrestore(&dev->req_lock, flags);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			defer_kevent(dev, WORK_RX_MEMORY);
			return;
		}

		spin_lock_irqsave(&dev->req_lock, flags);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);
}
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
static void process_uether_rx(struct eth_dev *dev)
{
=======

static __be16 ether_ip_type_trans(struct sk_buff *skb,
	struct net_device *dev)
{
	__be16	protocol = 0;

	skb->dev = dev;

	switch (skb->data[0] & 0xf0) {
	case 0x40:
		protocol = htons(ETH_P_IP);
		break;
	case 0x60:
		protocol = htons(ETH_P_IPV6);
		break;
	default:
		if ((skb->data[0] & 0x40) == 0x00)
			protocol = htons(ETH_P_MAP);
		else
			pr_debug_ratelimited("[%s] L3 protocol decode error: 0x%02x",
					dev->name, skb->data[0] & 0xf0);
	}

	return protocol;
}

static void process_rx_w(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, rx_work);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	struct sk_buff	*skb;
	int		status = 0;

	if (!dev->port_usb)
		return;

<<<<<<< HEAD
	while ((skb = skb_dequeue(&dev->rx_frames))) {
		if (status < 0
				|| ETH_HLEN > skb->len
				|| skb->len > ETH_FRAME_LEN) {
#ifdef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
		/*
		  Need to revisit net->mtu	does not include header size incase of changed MTU
		*/
			if(!strcmp(dev->port_usb->func.name,"ncm")) {
				if (status < 0
					|| ETH_HLEN > skb->len
					|| skb->len > (dev->net->mtu + ETH_HLEN)) {
					printk(KERN_ERR "usb: %s  drop incase of NCM rx length %d\n",__func__,skb->len);
				} else {
					printk(KERN_ERR "usb: %s  Dont drop incase of NCM rx length %d\n",__func__,skb->len);
					goto process_frame;
				}
			}
#endif
			dev->net->stats.rx_errors++;
			dev->net->stats.rx_length_errors++;
#ifndef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
			DBG(dev, "rx length %d\n", skb->len);
#else
			printk(KERN_DEBUG "usb: %s Drop rx length %d\n",__func__,skb->len);
#endif

=======
	set_wake_up_idle(true);
	while ((skb = skb_dequeue(&dev->rx_frames))) {
		if (status < 0
				|| ETH_HLEN > skb->len
				|| (skb->len > ETH_FRAME_LEN &&
				test_bit(RMNET_MODE_LLP_ETH, &dev->flags))) {
			dev->net->stats.rx_errors++;
			dev->net->stats.rx_length_errors++;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			DBG(dev, "rx length %d\n", skb->len);
			dev_kfree_skb_any(skb);
			continue;
		}
<<<<<<< HEAD
#ifdef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
process_frame:
#endif
		skb->protocol = eth_type_trans(skb, dev->net);
		dev->net->stats.rx_packets++;
		dev->net->stats.rx_bytes += skb->len;

		if (in_interrupt())
			status = netif_rx(skb);
		else
			status = netif_rx_ni(skb);
	}

	if (netif_running(dev->net))
		rx_fill(dev, GFP_ATOMIC);
}
static inline void process_rx_task(unsigned long data)
{
	struct eth_dev *dev = (struct eth_dev *)data;
	process_uether_rx(dev);
}
#endif
=======
		if (test_bit(RMNET_MODE_LLP_IP, &dev->flags))
			skb->protocol = ether_ip_type_trans(skb, dev->net);
		else
			skb->protocol = eth_type_trans(skb, dev->net);

		dev->net->stats.rx_packets++;
		dev->net->stats.rx_bytes += skb->len;

		status = netif_rx_ni(skb);
	}
	set_wake_up_idle(false);

	if (netif_running(dev->net))
		rx_fill(dev, GFP_KERNEL);
}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

static void eth_work(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, work);

	if (test_and_clear_bit(WORK_RX_MEMORY, &dev->todo)) {
		if (netif_running(dev->net))
			rx_fill(dev, GFP_KERNEL);
	}

	if (dev->todo)
		DBG(dev, "work done, flags = 0x%lx\n", dev->todo);
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
<<<<<<< HEAD
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	struct net_device *net;
	struct usb_request *new_req;
	struct usb_ep *in;
=======
	struct sk_buff	*skb;
	struct eth_dev	*dev;
	struct net_device *net;
	struct usb_request *new_req;
	struct usb_ep *in;
	int n = 1;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	int length;
	int retval;

	if (!ep->driver_data) {
<<<<<<< HEAD
		pr_err("%s: driver_data is null\n", __func__);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		usb_ep_free_request(ep, req);
		return;
	}

	dev = ep->driver_data;
	net = dev->net;

	if (!dev->port_usb) {
<<<<<<< HEAD
		pr_err("%s: port_usb is null\n", __func__);
		usb_ep_free_request(ep, req);
		return;
	}
#endif
=======
		usb_ep_free_request(ep, req);
		return;
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	switch (req->status) {
	default:
		dev->net->stats.tx_errors++;
		VDBG(dev, "tx err %d\n", req->status);
<<<<<<< HEAD
#ifdef CONFIG_USB_NCM_SUPPORT_MTU_CHANGE
		printk(KERN_ERR"usb:%s tx err %d\n",__func__, req->status);
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		if (!req->zero)
			dev->net->stats.tx_bytes += req->length-1;
		else
			dev->net->stats.tx_bytes += req->length;
#else
		dev->net->stats.tx_bytes += skb->len;
#endif
	}
	dev->net->stats.tx_packets++;

	spin_lock(&dev->req_lock);
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	list_add_tail(&req->list, &dev->tx_reqs);

	if (dev->port_usb->multi_pkt_xfer) {
=======
		if (!req->zero)
			dev->net->stats.tx_bytes += req->actual-1;
		else
			dev->net->stats.tx_bytes += req->actual;
	}

	if (req->num_sgs) {
		struct sg_ctx *sg_ctx = req->context;

		n = skb_queue_len(&sg_ctx->skbs);
		dev->tx_aggr_cnt[n-1]++;

		/* sg_ctx is only accessible here, can use lock-free version */
		__skb_queue_purge(&sg_ctx->skbs);
	}

	dev->net->stats.tx_packets += n;

	spin_lock(&dev->req_lock);
	list_add_tail(&req->list, &dev->tx_reqs);

	if (req->num_sgs) {
		if (!req->status)
			queue_work(uether_tx_wq, &dev->tx_work);

		spin_unlock(&dev->req_lock);
		return;
	}

	if (dev->port_usb->multi_pkt_xfer && !req->context) {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		dev->no_tx_req_used--;
		req->length = 0;
		in = dev->port_usb->in_ep;

		if (!list_empty(&dev->tx_reqs)) {
			new_req = container_of(dev->tx_reqs.next,
					struct usb_request, list);
			list_del(&new_req->list);
			spin_unlock(&dev->req_lock);
			if (new_req->length > 0) {
				length = new_req->length;

				/* NCM requires no zlp if transfer is
				 * dwNtbInMaxSize */
				if (dev->port_usb->is_fixed &&
					length == dev->port_usb->fixed_in_len &&
					(length % in->maxpacket) == 0)
					new_req->zero = 0;
				else
					new_req->zero = 1;

				/* use zlp framing on tx for strict CDC-Ether
				 * conformance, though any robust network rx
				 * path ignores extra padding. and some hardware
				 * doesn't like to write zlps.
				 */
				if (new_req->zero && !dev->zlp &&
						(length % in->maxpacket) == 0) {
					new_req->zero = 0;
					length++;
				}

				new_req->length = length;
<<<<<<< HEAD
				new_req->complete = tx_complete;
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
				retval = usb_ep_queue(in, new_req, GFP_ATOMIC);
				switch (retval) {
				default:
					DBG(dev, "tx queue err %d\n", retval);
					new_req->length = 0;
					spin_lock(&dev->req_lock);
<<<<<<< HEAD
					list_add_tail(&new_req->list, &dev->tx_reqs);
=======
					list_add_tail(&new_req->list,
							&dev->tx_reqs);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
					spin_unlock(&dev->req_lock);
					break;
				case 0:
					spin_lock(&dev->req_lock);
					dev->no_tx_req_used++;
					spin_unlock(&dev->req_lock);
					net->trans_start = jiffies;
				}
			} else {
				spin_lock(&dev->req_lock);
<<<<<<< HEAD
=======
				/*
				 * Put the idle request at the back of the
				 * queue. The xmit function will put the
				 * unfinished request at the beginning of the
				 * queue.
				 */
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
				list_add_tail(&new_req->list, &dev->tx_reqs);
				spin_unlock(&dev->req_lock);
			}
		} else {
			spin_unlock(&dev->req_lock);
		}
	} else {
<<<<<<< HEAD
		spin_unlock(&dev->req_lock);
		dev_kfree_skb_any(skb);
	}
#else
	list_add(&req->list, &dev->tx_reqs);
	spin_unlock(&dev->req_lock);
	dev_kfree_skb_any(skb);

	atomic_dec(&dev->tx_qlen);
#endif
=======
		skb = req->context;
		/* Is aggregation already enabled and buffers allocated ? */
		if (dev->port_usb->multi_pkt_xfer && dev->tx_req_bufsize) {
			req->buf = kzalloc(dev->tx_req_bufsize, GFP_ATOMIC);
			req->context = NULL;
		} else {
			req->buf = NULL;
		}

		spin_unlock(&dev->req_lock);
		dev_kfree_skb_any(skb);
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (netif_carrier_ok(dev->net))
		netif_wake_queue(dev->net);
}

static inline int is_promisc(u16 cdc_filter)
{
	return cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static int alloc_tx_buffer(struct eth_dev *dev)
{
	struct list_head	*act;
	struct usb_request	*req;

	dev->tx_req_bufsize = (dev->dl_max_pkts_per_xfer *
				(dev->net->mtu
				+ sizeof(struct ethhdr)
				/* size of rndis_packet_msg_type */
				+ 44
				+ 22));

	list_for_each(act, &dev->tx_reqs) {
		req = container_of(act, struct usb_request, list);
<<<<<<< HEAD
		if (!req->buf)
			req->buf = kmalloc(dev->tx_req_bufsize,
						GFP_ATOMIC);
			if (!req->buf)
				goto free_buf;
=======
		if (!req->buf) {
			req->buf = kzalloc(dev->tx_req_bufsize,
						GFP_ATOMIC);
			if (!req->buf)
				goto free_buf;
		}
		/* req->context is not used for multi_pkt_xfers */
		req->context = NULL;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}
	return 0;

free_buf:
	/* tx_req_bufsize = 0 retries mem alloc on next eth_start_xmit */
	dev->tx_req_bufsize = 0;
	list_for_each(act, &dev->tx_reqs) {
		req = container_of(act, struct usb_request, list);
		kfree(req->buf);
		req->buf = NULL;
	}
	return -ENOMEM;
<<<<<<< HEAD

}
#endif
=======
}

static void process_tx_w(struct work_struct *w)
{
	struct eth_dev		*dev = container_of(w, struct eth_dev, tx_work);
	struct net_device	*net = NULL;
	struct sk_buff		*skb = NULL;
	struct sg_ctx		*sg_ctx;
	struct usb_request	*req;
	struct usb_ep		*in = NULL;
	int			ret, count, hlen = 0, hdr_offset;
	uint32_t		max_size = 0;
	uint32_t		max_num_pkts = 1;
	unsigned long		flags;
	bool			header_on = false;
	int			req_cnt = 0;
	bool			port_usb_active;

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
		max_size = dev->dl_max_xfer_size;
		max_num_pkts = dev->dl_max_pkts_per_xfer;
		if (!max_num_pkts)
			max_num_pkts = 1;
		hlen = dev->header_len;
		net = dev->net;
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	spin_lock_irqsave(&dev->req_lock, flags);
	while (in && !list_empty(&dev->tx_reqs) &&
			(skb = skb_dequeue(&dev->tx_skb_q))) {
		req = list_first_entry(&dev->tx_reqs, struct usb_request,
				list);
		list_del(&req->list);
		spin_unlock_irqrestore(&dev->req_lock, flags);

		req->num_sgs = 0;
		req->zero = 1;
		req->length = 0;
		sg_ctx = req->context;
		skb_queue_head_init(&sg_ctx->skbs);
		sg_init_table(req->sg, DL_MAX_PKTS_PER_XFER);

		hdr_offset = 0;
		count = 1;
		do {
			/* spinlock can be avoided if buffer can passed
			 * wrap callback argument. However, it requires
			 * changes to all existing clients
			 */
			spin_lock_irqsave(&dev->lock, flags);
			if (!dev->port_usb) {
				spin_unlock_irqrestore(&dev->lock, flags);
				skb_queue_purge(&sg_ctx->skbs);
				kfree(req->sg);
				kfree(req->context);
				kfree(req->buf);
				usb_ep_free_request(in, req);

				return;
			}

			if (hlen && dev->wrap) {
				dev->port_usb->header = req->buf + hdr_offset;
				skb = dev->wrap(dev->port_usb, skb);
				header_on = true;
			}
			spin_unlock_irqrestore(&dev->lock, flags);

			if (header_on) {
				sg_set_buf(&req->sg[req->num_sgs],
					req->buf + hdr_offset, hlen);
				req->num_sgs++;
				hdr_offset += hlen;
				req->length += hlen;
			}

			/* skb processing */
			sg_set_buf(&req->sg[req->num_sgs], skb->data, skb->len);
			req->num_sgs++;

			req->length += skb->len;
			skb_queue_tail(&sg_ctx->skbs, skb);

			skb = skb_dequeue(&dev->tx_skb_q);
			if (!skb)
				break;
			if ((req->length + skb->len + hlen) >= max_size ||
					count >= max_num_pkts) {
				skb_queue_head(&dev->tx_skb_q, skb);
				break;
			}
			count++;
		} while (true);

		sg_mark_end(&req->sg[req->num_sgs - 1]);

		spin_lock_irqsave(&dev->lock, flags);
		if (dev->port_usb) {
			in = dev->port_usb->in_ep;
			port_usb_active = 1;
		} else {
			port_usb_active = 0;
		}
		spin_unlock_irqrestore(&dev->lock, flags);

		if (!port_usb_active) {
			__skb_queue_purge(&sg_ctx->skbs);
			kfree(req->sg);
			kfree(req->context);
			kfree(req->buf);
			usb_ep_free_request(in, req);

			return;
		}

		ret = usb_ep_queue(in, req, GFP_KERNEL);
		spin_lock_irqsave(&dev->req_lock, flags);
		switch (ret) {
		default:
			dev->net->stats.tx_dropped +=
				skb_queue_len(&sg_ctx->skbs);

			__skb_queue_purge(&sg_ctx->skbs);
			list_add_tail(&req->list, &dev->tx_reqs);
			break;
		case 0:
			net->trans_start = jiffies;
		}

		/* break the loop after processing 10 packets
		 * otherwise wd may kick in
		 */
		if (ret || ++req_cnt > 10) {
			dev->loop_brk_cnt++;
			break;
		}

		if (dev->tx_skb_q.qlen <  tx_start_threshold)
			netif_start_queue(net);

	}
	spin_unlock_irqrestore(&dev->req_lock, flags);
}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

static netdev_tx_t eth_start_xmit(struct sk_buff *skb,
					struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = skb->len;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags;
<<<<<<< HEAD
	struct usb_ep		*in;
	u16			cdc_filter;
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	bool			multi_pkt_xfer = false;
#endif
=======
	struct usb_ep		*in = NULL;
	u16			cdc_filter = 0;
	bool			multi_pkt_xfer = false;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
		cdc_filter = dev->port_usb->cdc_filter;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		multi_pkt_xfer = dev->port_usb->multi_pkt_xfer;
#endif
	} else {
		in = NULL;
		cdc_filter = 0;
=======
		multi_pkt_xfer = dev->port_usb->multi_pkt_xfer;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!in) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	/* Allocate memory for tx_reqs to support multi packet transfer */
	if (multi_pkt_xfer && !dev->tx_req_bufsize) {
		retval = alloc_tx_buffer(dev);
		if (retval < 0)
			return -ENOMEM;
	}
#endif

	/* apply outgoing CDC or RNDIS filters */
	if (!is_promisc(cdc_filter)) {
=======
	/* apply outgoing CDC or RNDIS filters only for ETH packets */
	if (!test_bit(RMNET_MODE_LLP_IP, &dev->flags) &&
						!is_promisc(cdc_filter)) {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		u8		*dest = skb->data;

		if (is_multicast_ether_addr(dest)) {
			u16	type;

			/* ignores USB_CDC_PACKET_TYPE_MULTICAST and host
			 * SET_ETHERNET_MULTICAST_FILTERS requests
			 */
			if (is_broadcast_ether_addr(dest))
				type = USB_CDC_PACKET_TYPE_BROADCAST;
			else
				type = USB_CDC_PACKET_TYPE_ALL_MULTICAST;
			if (!(cdc_filter & type)) {
				dev_kfree_skb_any(skb);
				return NETDEV_TX_OK;
			}
		}
		/* ignores USB_CDC_PACKET_TYPE_DIRECTED */
	}

<<<<<<< HEAD
	spin_lock_irqsave(&dev->req_lock, flags);
=======
	dev->tx_pkts_rcvd++;
	if (dev->gadget->sg_supported) {
		skb_queue_tail(&dev->tx_skb_q, skb);
		if (dev->tx_skb_q.qlen > tx_stop_threshold) {
			dev->tx_throttle++;
			netif_stop_queue(net);
		}

		queue_work(uether_tx_wq, &dev->tx_work);
		return NETDEV_TX_OK;
	}

	/*
	 * No buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers or there's not enough
	 * enough space for extra headers we need.
	 */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->wrap && dev->port_usb)
		skb = dev->wrap(dev->port_usb, skb);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!skb) {
		dev->net->stats.tx_dropped++;

		/* no error code for dropped packets */
		return NETDEV_TX_OK;
	}

	/* Allocate memory for tx_reqs to support multi packet transfer */
	spin_lock_irqsave(&dev->req_lock, flags);
	if (multi_pkt_xfer && !dev->tx_req_bufsize) {
		retval = alloc_tx_buffer(dev);
		if (retval < 0) {
			spin_unlock_irqrestore(&dev->req_lock, flags);
			return -ENOMEM;
		}
	}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/*
	 * this freelist can be empty if an interrupt triggered disconnect()
	 * and reconfigured the gadget (shutting down this queue) after the
	 * network stack decided to xmit but before we got the spinlock.
	 */
	if (list_empty(&dev->tx_reqs)) {
		spin_unlock_irqrestore(&dev->req_lock, flags);
		return NETDEV_TX_BUSY;
	}

	req = container_of(dev->tx_reqs.next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
<<<<<<< HEAD
	if (list_empty(&dev->tx_reqs))
		netif_stop_queue(net);
	spin_unlock_irqrestore(&dev->req_lock, flags);

	/* no buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers.
	 * or there's not enough space for extra headers we need
	 */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->wrap) {
		if (dev->port_usb)
			skb = dev->wrap(dev->port_usb, skb);
		if (!skb) {
			spin_unlock_irqrestore(&dev->lock, flags);
			goto drop;
		}
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	}
=======
	if (list_empty(&dev->tx_reqs)) {
		/*
		 * tx_throttle gives info about number of times u_ether
		 * asked network layer to stop queueing packets to it
		 * when transmit resources are unavailable
		 */
		dev->tx_throttle++;
		netif_stop_queue(net);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (multi_pkt_xfer) {

		pr_debug("req->length:%d header_len:%u\n"
				"skb->len:%d skb->data_len:%d\n",
				req->length, dev->header_len,
				skb->len, skb->data_len);
		/* Add RNDIS Header */
		memcpy(req->buf + req->length, dev->port_usb->header,
						dev->header_len);
		/* Increment req length by header size */
		req->length += dev->header_len;
<<<<<<< HEAD
		spin_unlock_irqrestore(&dev->lock, flags);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		/* Copy received IP data from SKB */
		memcpy(req->buf + req->length, skb->data, skb->len);
		/* Increment req length by skb data length */
		req->length += skb->len;
		length = req->length;
		dev_kfree_skb_any(skb);

		spin_lock_irqsave(&dev->req_lock, flags);
		dev->tx_skb_hold_count++;
		if (dev->tx_skb_hold_count < dev->dl_max_pkts_per_xfer) {
			if (dev->no_tx_req_used > TX_REQ_THRESHOLD) {
				list_add(&req->list, &dev->tx_reqs);
				spin_unlock_irqrestore(&dev->req_lock, flags);
				goto success;
			}
		}

		dev->no_tx_req_used++;
		dev->tx_skb_hold_count = 0;
		spin_unlock_irqrestore(&dev->req_lock, flags);
	} else {
<<<<<<< HEAD
		spin_unlock_irqrestore(&dev->lock, flags);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		length = skb->len;
		req->buf = skb->data;
		req->context = skb;
	}
<<<<<<< HEAD
#else
		length = skb->len;
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	req->buf = skb->data;
	req->context = skb;
#endif
	req->complete = tx_complete;
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/* NCM requires no zlp if transfer is dwNtbInMaxSize */
	if (dev->port_usb->is_fixed &&
	    length == dev->port_usb->fixed_in_len &&
	    (length % in->maxpacket) == 0)
		req->zero = 0;
	else
		req->zero = 1;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	if (req->zero && !dev->zlp && (length % in->maxpacket) == 0) {
		req->zero = 0;
		length++;
	}

	req->length = length;

<<<<<<< HEAD
	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget))
		req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH ||
				     dev->gadget->speed == USB_SPEED_SUPER)
			? ((atomic_read(&dev->tx_qlen) % qmult) != 0)
					: 0;
=======
	/* throttle high/super speed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget) &&
		 (dev->gadget->speed == USB_SPEED_HIGH ||
		  dev->gadget->speed == USB_SPEED_SUPER)) {
		dev->tx_qlen++;
		if (dev->tx_qlen == (qmult/2)) {
			req->no_interrupt = 0;
			dev->tx_qlen = 0;
		} else {
			req->no_interrupt = 1;
		}
	} else {
		req->no_interrupt = 0;
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	retval = usb_ep_queue(in, req, GFP_ATOMIC);
	switch (retval) {
	default:
		DBG(dev, "tx queue err %d\n", retval);
		break;
	case 0:
		net->trans_start = jiffies;
<<<<<<< HEAD
		atomic_inc(&dev->tx_qlen);
	}

	if (retval) {
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
=======
	}

	if (retval) {
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (!multi_pkt_xfer)
			dev_kfree_skb_any(skb);
		else
			req->length = 0;
<<<<<<< HEAD
#else
		dev_kfree_skb_any(skb);
#endif
drop:
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		dev->net->stats.tx_dropped++;
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty(&dev->tx_reqs))
			netif_start_queue(net);
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		list_add_tail(&req->list, &dev->tx_reqs);
#else
		list_add(&req->list, &dev->tx_reqs);
#endif
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
success:
#endif
=======
		list_add_tail(&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
success:
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return NETDEV_TX_OK;
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
	DBG(dev, "%s\n", __func__);

	/* fill the rx queue */
	rx_fill(dev, gfp_flags);

	/* and open the tx floodgates */
<<<<<<< HEAD
	atomic_set(&dev->tx_qlen, 0);
=======
	dev->tx_qlen = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	netif_wake_queue(dev->net);
}

static int eth_open(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	struct gether	*link;
<<<<<<< HEAD

	DBG(dev, "%s\n", __func__);
	if (netif_carrier_ok(dev->net))
		eth_start(dev, GFP_KERNEL);

	spin_lock_irq(&dev->lock);
	link = dev->port_usb;
=======
	bool wait_for_rx_trigger;
	int i;

	DBG(dev, "%s\n", __func__);

	spin_lock_irq(&dev->lock);
	link = dev->port_usb;
	spin_unlock_irq(&dev->lock);

	wait_for_rx_trigger = dev->rx_trigger_enabled && link &&
		!link->rx_triggered;

	if (netif_carrier_ok(dev->net) && !wait_for_rx_trigger)
		eth_start(dev, GFP_KERNEL);

	dev->state = ETH_START;
	for_each_online_cpu(i)
		cpufreq_update_policy(i);

	spin_lock_irq(&dev->lock);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (link && link->open)
		link->open(link);
	spin_unlock_irq(&dev->lock);

	return 0;
}

static int eth_stop(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
<<<<<<< HEAD

	VDBG(dev, "%s\n", __func__);
=======
	int i;
	enum ifc_state prev_state;

	VDBG(dev, "%s\n", __func__);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	netif_stop_queue(net);

	DBG(dev, "stop stats: rx/tx %ld/%ld, errs %ld/%ld\n",
		dev->net->stats.rx_packets, dev->net->stats.tx_packets,
		dev->net->stats.rx_errors, dev->net->stats.tx_errors
		);

	/* ensure there are no more active requests */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		struct gether	*link = dev->port_usb;
		const struct usb_endpoint_descriptor *in;
		const struct usb_endpoint_descriptor *out;

		if (link->close)
			link->close(link);

		/* NOTE:  we have no abort-queue primitive we could use
		 * to cancel all pending I/O.  Instead, we disable then
		 * reenable the endpoints ... this idiom may leave toggle
		 * wrong, but that's a self-correcting error.
		 *
		 * REVISIT:  we *COULD* just let the transfers complete at
		 * their own pace; the network stack can handle old packets.
		 * For the moment we leave this here, since it works.
		 */
		in = link->in_ep->desc;
		out = link->out_ep->desc;
		usb_ep_disable(link->in_ep);
		usb_ep_disable(link->out_ep);
		if (netif_carrier_ok(net)) {
<<<<<<< HEAD
=======
			if (config_ep_by_speed(dev->gadget, &link->func,
					       link->in_ep) ||
			    config_ep_by_speed(dev->gadget, &link->func,
					       link->out_ep)) {
				link->in_ep->desc = NULL;
				link->out_ep->desc = NULL;
				return -EINVAL;
			}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			DBG(dev, "host still using in/out endpoints\n");
			link->in_ep->desc = in;
			link->out_ep->desc = out;
			usb_ep_enable(link->in_ep);
			usb_ep_enable(link->out_ep);
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

<<<<<<< HEAD
=======
	prev_state = dev->state;
	dev->state = ETH_STOP;

	/* if previous state is eth_start, update cpufreq policy to normal */
	if (prev_state == ETH_START)
		for_each_online_cpu(i)
			cpufreq_update_policy(i);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return 0;
}

/*-------------------------------------------------------------------------*/

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *dev_addr;
module_param(dev_addr, charp, S_IRUGO);
MODULE_PARM_DESC(dev_addr, "Device Ethernet Address");

/* this address is invisible to ifconfig */
static char *host_addr;
module_param(host_addr, charp, S_IRUGO);
MODULE_PARM_DESC(host_addr, "Host Ethernet Address");

static int get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	eth_random_addr(dev_addr);
	return 1;
}

<<<<<<< HEAD
=======
static int ether_ioctl(struct net_device *, struct ifreq *, int);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static const struct net_device_ops eth_netdev_ops = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
<<<<<<< HEAD
=======
	.ndo_do_ioctl		= ether_ioctl,
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	.ndo_change_mtu		= ueth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

<<<<<<< HEAD
=======
static const struct net_device_ops eth_netdev_ops_ip = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
	.ndo_do_ioctl		= ether_ioctl,
	.ndo_change_mtu		= ueth_change_mtu_ip,
	.ndo_set_mac_address	= 0,
	.ndo_validate_addr	= 0,
};

static int rmnet_ioctl_extended(struct net_device *dev, struct ifreq *ifr)
{
	struct rmnet_ioctl_extended_s ext_cmd;
	struct eth_dev *eth_dev = netdev_priv(dev);
	int rc = 0;

	rc = copy_from_user(&ext_cmd, ifr->ifr_ifru.ifru_data,
			    sizeof(struct rmnet_ioctl_extended_s));

	if (rc) {
		DBG("%s(): copy_from_user() failed\n", __func__);
		return rc;
	}

	switch (ext_cmd.extended_ioctl) {
	case RMNET_IOCTL_GET_SUPPORTED_FEATURES:
		ext_cmd.u.data = 0;
		break;

	case RMNET_IOCTL_SET_MRU:
		if (netif_running(dev))
			return -EBUSY;

		/* 16K max */
		if ((size_t)ext_cmd.u.data > 0x4000)
			return -EINVAL;

		if (eth_dev->port_usb) {
			eth_dev->port_usb->is_fixed = true;
			eth_dev->port_usb->fixed_out_len =
				(size_t) ext_cmd.u.data;
			DBG("[%s] rmnet_ioctl(): SET MRU to %u\n", dev->name,
				eth_dev->mru);
		} else {
			pr_err("[%s]: %s: SET MRU failed. Cable disconnected\n",
				dev->name, __func__);
			return -ENODEV;
		}
		break;

	case RMNET_IOCTL_GET_MRU:
		if (eth_dev->port_usb) {
			ext_cmd.u.data = eth_dev->port_usb->is_fixed ?
					eth_dev->port_usb->fixed_out_len :
					dev->mtu;
		} else {
			pr_err("[%s]: %s: GET MRU failed. Cable disconnected\n",
				dev->name, __func__);
			return -ENODEV;
		}
		break;

	case RMNET_IOCTL_GET_DRIVER_NAME:
		strlcpy(ext_cmd.u.if_name, dev->name,
			sizeof(ext_cmd.u.if_name));
		break;

	default:
		break;
	}

	rc = copy_to_user(ifr->ifr_ifru.ifru_data, &ext_cmd,
			  sizeof(struct rmnet_ioctl_extended_s));

	if (rc)
		DBG("%s(): copy_to_user() failed\n", __func__);
	return rc;
}

static int ether_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct eth_dev	*eth_dev = netdev_priv(dev);
	void __user *addr = (void __user *) ifr->ifr_ifru.ifru_data;
	int		prev_mtu = dev->mtu;
	u32		state, old_opmode;
	int		rc = -EFAULT;

	old_opmode = eth_dev->flags;
	/* Process IOCTL command */
	switch (cmd) {
	case RMNET_IOCTL_SET_LLP_ETHERNET:	/*Set Ethernet protocol*/
		/* Perform Ethernet config only if in IP mode currently*/
		if (test_bit(RMNET_MODE_LLP_IP, &eth_dev->flags)) {
			ether_setup(dev);
			dev->mtu = prev_mtu;
			dev->netdev_ops = &eth_netdev_ops;
			clear_bit(RMNET_MODE_LLP_IP, &eth_dev->flags);
			set_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags);
			DBG(eth_dev, "[%s] ioctl(): set Ethernet proto mode\n",
					dev->name);
		}
		if (test_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags))
			rc = 0;
		break;

	case RMNET_IOCTL_SET_LLP_IP:		/* Set RAWIP protocol*/
		/* Perform IP config only if in Ethernet mode currently*/
		if (test_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags)) {
			/* Undo config done in ether_setup() */
			dev->header_ops = 0;  /* No header */
			dev->type = ARPHRD_RAWIP;
			dev->hard_header_len = 0;
			dev->mtu = prev_mtu;
			dev->addr_len = 0;
			dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
			dev->netdev_ops = &eth_netdev_ops_ip;
			clear_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags);
			set_bit(RMNET_MODE_LLP_IP, &eth_dev->flags);
			DBG(eth_dev, "[%s] ioctl(): set IP protocol mode\n",
					dev->name);
		}
		if (test_bit(RMNET_MODE_LLP_IP, &eth_dev->flags))
			rc = 0;
		break;

	case RMNET_IOCTL_GET_LLP:	/* Get link protocol state */
		state = eth_dev->flags & (RMNET_MODE_LLP_ETH
						| RMNET_MODE_LLP_IP);
		if (copy_to_user(addr, &state, sizeof(state)))
			break;
		rc = 0;
		break;

	case RMNET_IOCTL_SET_RX_HEADROOM:	/* Set RX headroom */
		if (copy_from_user(&eth_dev->rx_needed_headroom, addr,
					sizeof(eth_dev->rx_needed_headroom)))
			break;
		DBG(eth_dev, "[%s] ioctl(): set RX HEADROOM: %x\n",
				dev->name, eth_dev->rx_needed_headroom);
		rc = 0;
		break;

	case RMNET_IOCTL_EXTENDED:
		rc = rmnet_ioctl_extended(dev, ifr);
		break;

	default:
		pr_err("[%s] error: ioctl called for unsupported cmd[%d]",
			dev->name, cmd);
		rc = -EINVAL;
	}

	DBG(eth_dev, "[%s] %s: cmd=0x%x opmode old=0x%08x new=0x%08lx\n",
		dev->name, __func__, cmd, old_opmode, eth_dev->flags);

	return rc;
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
static struct device_type gadget_type = {
	.name	= "gadget",
};

<<<<<<< HEAD
=======
static int gether_cpufreq_notifier_cb(struct notifier_block *nfb,
		unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;
	unsigned int cpu = policy->cpu;
	struct eth_dev	*dev = container_of(nfb, struct eth_dev,
					cpufreq_notifier);

	if (!min_cpu_freq)
		return NOTIFY_OK;

	switch (event) {
	case CPUFREQ_ADJUST:
		pr_debug("%s: cpu:%u\n", __func__, cpu);

		if (dev->state == ETH_START)
			cpufreq_verify_within_limits(policy,
					min_cpu_freq, UINT_MAX);

		break;
	}

	return NOTIFY_OK;
}

static void update_cpu_policy_w(struct work_struct *work)
{
	int i;

	for_each_online_cpu(i)
		cpufreq_update_policy(i);
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/**
 * gether_setup_name - initialize one ethernet-over-usb link
 * @g: gadget to associated with these links
 * @ethaddr: NULL, or a buffer in which the ethernet address of the
 *	host side of the link is recorded
 * @netname: name for network device (for example, "usb")
 * Context: may sleep
 *
 * This sets up the single network link that may be exported by a
 * gadget driver using this framework.  The link layer addresses are
 * set up using module parameters.
 *
 * Returns negative errno, or zero on success
 */
struct eth_dev *gether_setup_name(struct usb_gadget *g, u8 ethaddr[ETH_ALEN],
		const char *netname)
{
	struct eth_dev		*dev;
	struct net_device	*net;
	int			status;

	net = alloc_etherdev(sizeof *dev);
	if (!net)
		return ERR_PTR(-ENOMEM);

	dev = netdev_priv(net);
	spin_lock_init(&dev->lock);
	spin_lock_init(&dev->req_lock);
	INIT_WORK(&dev->work, eth_work);
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	tasklet_init(&dev->rx_tsk, process_rx_task, (unsigned long)dev);
#endif
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);

	skb_queue_head_init(&dev->rx_frames);
=======
	INIT_WORK(&dev->rx_work, process_rx_w);
	INIT_WORK(&dev->tx_work, process_tx_w);
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);
	INIT_WORK(&dev->cpu_policy_w, update_cpu_policy_w);

	skb_queue_head_init(&dev->rx_frames);
	skb_queue_head_init(&dev->tx_skb_q);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	/* network device setup */
	dev->net = net;
	snprintf(net->name, sizeof(net->name), "%s%%d", netname);

	if (get_ether_addr(dev_addr, net->dev_addr))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "self");
<<<<<<< HEAD
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
	memcpy(dev->host_mac, ethaddr, ETH_ALEN);
	printk(KERN_DEBUG "usb: set unique host mac\n");
#else
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (get_ether_addr(host_addr, dev->host_mac))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "host");

	if (ethaddr)
		memcpy(ethaddr, dev->host_mac, ETH_ALEN);
<<<<<<< HEAD
#endif
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	net->netdev_ops = &eth_netdev_ops;

	SET_ETHTOOL_OPS(net, &ops);

<<<<<<< HEAD
=======
	/* set operation mode to eth by default */
	set_bit(RMNET_MODE_LLP_ETH, &dev->flags);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	dev->gadget = g;
	SET_NETDEV_DEV(net, &g->dev);
	SET_NETDEV_DEVTYPE(net, &gadget_type);

	status = register_netdev(net);
	if (status < 0) {
		dev_dbg(&g->dev, "register_netdev failed, %d\n", status);
		free_netdev(net);
		dev = ERR_PTR(status);
	} else {
		INFO(dev, "MAC %pM\n", net->dev_addr);
		INFO(dev, "HOST MAC %pM\n", dev->host_mac);

		/* two kinds of host-initiated state changes:
		 *  - iff DATA transfer is active, carrier is "on"
		 *  - tx queueing enabled if open *and* carrier is "on"
		 */
		netif_carrier_off(net);
<<<<<<< HEAD
=======
		uether_debugfs_init(dev, netname);

		dev->cpufreq_notifier.notifier_call =
					gether_cpufreq_notifier_cb;
		cpufreq_register_notifier(&dev->cpufreq_notifier,
				CPUFREQ_POLICY_NOTIFIER);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	}

	return dev;
}

/**
 * gether_cleanup - remove Ethernet-over-USB device
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gether_setup().
 */
void gether_cleanup(struct eth_dev *dev)
{
<<<<<<< HEAD
	if (!dev)
		return;

#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	tasklet_kill(&dev->rx_tsk);
#endif
=======
	int i;

	if (!dev)
		return;

	/* make sure cpu boost is set to normal again */
	dev->state = ETH_UNDEFINED;
	cancel_work_sync(&dev->cpu_policy_w);
	for_each_online_cpu(i)
		cpufreq_update_policy(i);

	cpufreq_unregister_notifier(&dev->cpufreq_notifier,
				CPUFREQ_POLICY_NOTIFIER);

	uether_debugfs_exit(dev);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	unregister_netdev(dev->net);
	flush_work(&dev->work);
	free_netdev(dev->net);
}

<<<<<<< HEAD
=======
void gether_update_dl_max_xfer_size(struct gether *link, uint32_t s)
{
	struct eth_dev		*dev = link->ioport;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	dev->dl_max_xfer_size = s;
	spin_unlock_irqrestore(&dev->lock, flags);
}

void gether_update_dl_max_pkts_per_xfer(struct gether *link, uint32_t n)
{
	struct eth_dev		*dev = link->ioport;
	unsigned long flags;

	if (n > DL_MAX_PKTS_PER_XFER)
		n = DL_MAX_PKTS_PER_XFER;

	spin_lock_irqsave(&dev->lock, flags);
	dev->dl_max_pkts_per_xfer = n;
	spin_unlock_irqrestore(&dev->lock, flags);
}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/**
 * gether_connect - notify network layer that USB link is active
 * @link: the USB link, set up with endpoints, descriptors matching
 *	current device speed, and any framing wrapper(s) set up.
 * Context: irqs blocked
 *
 * This is called to activate endpoints and let the network layer know
 * the connection is active ("carrier detect").  It may cause the I/O
 * queues to open and start letting network packets flow, but will in
 * any case activate the endpoints so that they respond properly to the
 * USB host.
 *
 * Verify net_device pointer returned using IS_ERR().  If it doesn't
 * indicate some error code (negative errno), ep->driver_data values
 * have been overwritten.
 */
struct net_device *gether_connect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	int			result = 0;
<<<<<<< HEAD
=======
	bool wait_for_rx_trigger;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	if (!dev)
		return ERR_PTR(-EINVAL);

<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	link->header = kzalloc(sizeof(struct rndis_packet_msg_type),
							GFP_ATOMIC);
	if (!link->header) {
		pr_err("RNDIS header memory allocation failed.\n");
		result = -ENOMEM;
		goto fail;
	}
#endif
=======
	/* if scatter/gather or sg is supported then headers can be part of
	 * req->buf which is allocated later
	 */
	if (!dev->gadget->sg_supported) {
		link->header = kzalloc(sizeof(struct rndis_packet_msg_type),
						GFP_ATOMIC);
		if (!link->header) {
			pr_err("RNDIS header memory allocation failed.\n");
			result = -ENOMEM;
			goto fail;
		}
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	link->in_ep->driver_data = dev;
	result = usb_ep_enable(link->in_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->in_ep->name, result);
		goto fail0;
	}

	link->out_ep->driver_data = dev;
	result = usb_ep_enable(link->out_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->out_ep->name, result);
		goto fail1;
	}

<<<<<<< HEAD
=======
	dev->header_len = link->header_len;
	dev->unwrap = link->unwrap;
	dev->wrap = link->wrap;
	dev->ul_max_pkts_per_xfer = link->ul_max_pkts_per_xfer;
	dev->dl_max_pkts_per_xfer = link->dl_max_pkts_per_xfer;
	dev->dl_max_xfer_size = link->dl_max_xfer_size;
	dev->rx_trigger_enabled = link->rx_trigger_enabled;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (result == 0)
		result = alloc_requests(dev, link, qlen(dev->gadget));

	if (result == 0) {
<<<<<<< HEAD
		dev->zlp = link->is_zlp_ok;
		DBG(dev, "qlen %d\n", qlen(dev->gadget));

		dev->header_len = link->header_len;
		dev->unwrap = link->unwrap;
		dev->wrap = link->wrap;
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		dev->ul_max_pkts_per_xfer = link->ul_max_pkts_per_xfer;
		dev->dl_max_pkts_per_xfer = link->dl_max_pkts_per_xfer;
=======

		dev->zlp = link->is_zlp_ok;
		DBG(dev, "qlen %d\n", qlen(dev->gadget));

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		spin_lock(&dev->lock);
		dev->tx_skb_hold_count = 0;
		dev->no_tx_req_used = 0;
		dev->tx_req_bufsize = 0;
		dev->port_usb = link;
<<<<<<< HEAD
		link->ioport = dev;
#else
		spin_lock(&dev->lock);
		dev->port_usb = link;
#endif
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
		if (netif_running(dev->net)) {
			if (link->open)
				link->open(link);
		} else {
			if (link->close)
				link->close(link);
		}
		spin_unlock(&dev->lock);

		netif_carrier_on(dev->net);
<<<<<<< HEAD
		if (netif_running(dev->net))
=======

		wait_for_rx_trigger = dev->rx_trigger_enabled &&
			!link->rx_triggered;

		if (netif_running(dev->net) && !wait_for_rx_trigger)
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
			eth_start(dev, GFP_ATOMIC);

	/* on error, disable any endpoints  */
	} else {
		(void) usb_ep_disable(link->out_ep);
fail1:
		(void) usb_ep_disable(link->in_ep);
	}
<<<<<<< HEAD
fail0:
	/* caller is responsible for cleanup on error */
	if (result < 0) {
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	kfree(link->header);
fail:
#endif
		return ERR_PTR(result);
	}
=======

	/* caller is responsible for cleanup on error */
	if (result < 0) {
fail0:
		kfree(link->header);
fail:
		return ERR_PTR(result);
	}

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	return dev->net;
}

/**
 * gether_disconnect - notify network layer that USB link is inactive
 * @link: the USB link, on which gether_connect() was called
 * Context: irqs blocked
 *
 * This is called to deactivate endpoints and let the network layer know
 * the connection went inactive ("no carrier").
 *
 * On return, the state is as if gether_connect() had never been called.
 * The endpoints are inactive, and accordingly without active USB I/O.
 * Pointers to endpoint descriptors and endpoint private data are nulled.
 */
void gether_disconnect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	struct usb_request	*req;
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	struct sk_buff	*skb;
#endif

	WARN_ON(!dev);
=======
	struct sk_buff		*skb;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	if (!dev)
		return;

	DBG(dev, "%s\n", __func__);

<<<<<<< HEAD
=======
	dev->state = ETH_UNDEFINED;
	queue_work(uether_wq, &dev->cpu_policy_w);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	netif_stop_queue(dev->net);
	netif_carrier_off(dev->net);

	/* disable endpoints, forcing (synchronous) completion
	 * of all pending i/o.  then free the request objects
	 * and forget about the endpoints.
	 */
	usb_ep_disable(link->in_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->tx_reqs)) {
		req = container_of(dev->tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
<<<<<<< HEAD
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
		if (link->multi_pkt_xfer) {
			kfree(req->buf);
			req->buf = NULL;
		}
#endif
		usb_ep_free_request(link->in_ep, req);
		spin_lock(&dev->req_lock);
	}
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	/* Free rndis header buffer memory */
	kfree(link->header);
	link->header = NULL;
#endif

	spin_unlock(&dev->req_lock);
#ifdef CONFIG_USB_RNDIS_MULTIPACKET
	spin_lock(&dev->rx_frames.lock);
	while ((skb = __skb_dequeue(&dev->rx_frames))) {
		pr_info("%s: dequeuing skb and free\n", __func__);
		dev_kfree_skb_any(skb);
	}
	spin_unlock(&dev->rx_frames.lock);
#endif
=======
		if (link->multi_pkt_xfer ||
				dev->gadget->sg_supported) {
			kfree(req->buf);
			req->buf = NULL;
		}
		if (dev->gadget->sg_supported) {
			kfree(req->context);
			kfree(req->sg);
		}

		usb_ep_free_request(link->in_ep, req);
		spin_lock(&dev->req_lock);
	}

	/* Free rndis header buffer memory */
	if (!dev->gadget->sg_supported)
		kfree(link->header);
	link->header = NULL;
	spin_unlock(&dev->req_lock);

	skb_queue_purge(&dev->tx_skb_q);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	link->in_ep->driver_data = NULL;
	link->in_ep->desc = NULL;

	usb_ep_disable(link->out_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->rx_reqs)) {
		req = container_of(dev->rx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->out_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
<<<<<<< HEAD
	link->out_ep->driver_data = NULL;
	link->out_ep->desc = NULL;

=======

	spin_lock(&dev->rx_frames.lock);
	while ((skb = __skb_dequeue(&dev->rx_frames)))
		dev_kfree_skb_any(skb);
	spin_unlock(&dev->rx_frames.lock);

	link->out_ep->driver_data = NULL;
	link->out_ep->desc = NULL;

	pr_debug("%s(): tx_throttle count= %lu", __func__,
					dev->tx_throttle);
	/* reset tx_throttle count */
	dev->tx_throttle = 0;
	dev->rx_throttle = 0;

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	/* finish forgetting about this USB link episode */
	dev->header_len = 0;
	dev->unwrap = NULL;
	dev->wrap = NULL;
<<<<<<< HEAD
=======
	dev->rx_trigger_enabled = 0;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	spin_lock(&dev->lock);
	dev->port_usb = NULL;
	spin_unlock(&dev->lock);
}
<<<<<<< HEAD
=======

int gether_up(struct gether *link)
{
	struct eth_dev *dev = link->ioport;

	if (dev && netif_carrier_ok(dev->net))
		eth_start(dev, GFP_KERNEL);

	return 0;
}


static int uether_stat_show(struct seq_file *s, void *unused)
{
	struct eth_dev *dev = s->private;
	int ret = 0;
	int i;

	if (dev) {
		seq_printf(s, "rx_throttle = %lu\n",
					dev->rx_throttle);
		seq_printf(s, "tx_qlen=%u tx_throttle = %lu\n aggr count:",
					dev->tx_skb_q.qlen,
					dev->tx_throttle);
		for (i = 0; i < DL_MAX_PKTS_PER_XFER; i++)
			seq_printf(s, "%u\t", dev->tx_aggr_cnt[i]);

		seq_printf(s, "\nloop_brk_cnt = %u\n tx_pkts_rcvd=%u\n",
					dev->loop_brk_cnt,
					dev->tx_pkts_rcvd);
	}

	return ret;
}

static int uether_open(struct inode *inode, struct file *file)
{
	return single_open(file, uether_stat_show, inode->i_private);
}

static ssize_t uether_stat_reset(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct eth_dev *dev = s->private;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	/* Reset tx_throttle */
	dev->tx_throttle = 0;
	dev->rx_throttle = 0;
	spin_unlock_irqrestore(&dev->lock, flags);
	return count;
}

const struct file_operations uether_stats_ops = {
	.open = uether_open,
	.read = seq_read,
	.write = uether_stat_reset,
};

static void uether_debugfs_init(struct eth_dev *dev, const char *name)
{
	struct dentry *uether_dent;
	struct dentry *uether_dfile;

	uether_dent = debugfs_create_dir(name, 0);
	if (IS_ERR(uether_dent))
		return;
	dev->uether_dent = uether_dent;

	uether_dfile = debugfs_create_file("status", S_IRUGO | S_IWUSR,
				uether_dent, dev, &uether_stats_ops);
	if (!uether_dfile || IS_ERR(uether_dfile))
		debugfs_remove(uether_dent);
	dev->uether_dfile = uether_dfile;
}

static void uether_debugfs_exit(struct eth_dev *dev)
{
	debugfs_remove(dev->uether_dfile);
	debugfs_remove(dev->uether_dent);
	dev->uether_dent = NULL;
	dev->uether_dfile = NULL;
}

static int __init gether_init(void)
{
	uether_wq  = create_singlethread_workqueue("uether");
	if (!uether_wq) {
		pr_err("%s: Unable to create workqueue: uether\n", __func__);
		return -ENOMEM;
	}

	uether_tx_wq = alloc_workqueue("uether_tx",
				WQ_CPU_INTENSIVE | WQ_UNBOUND, 1);
	if (!uether_tx_wq) {
		destroy_workqueue(uether_wq);
		pr_err("%s: Unable to create workqueue: uether\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
module_init(gether_init);

static void __exit gether_exit(void)
{
	destroy_workqueue(uether_tx_wq);
	destroy_workqueue(uether_wq);

}
module_exit(gether_exit);
MODULE_DESCRIPTION("ethernet over USB driver");
MODULE_LICENSE("GPL v2");
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
