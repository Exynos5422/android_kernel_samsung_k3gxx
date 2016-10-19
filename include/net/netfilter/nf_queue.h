#ifndef _NF_QUEUE_H
#define _NF_QUEUE_H
<<<<<<< HEAD
=======

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
/* Each queued (to userspace) skbuff has one of these. */
struct nf_queue_entry {
	struct list_head	list;
	struct sk_buff		*skb;
	unsigned int		id;

	struct nf_hook_ops	*elem;
	u_int8_t		pf;
	u16			size; /* sizeof(entry) + saved route keys */
	unsigned int		hook;
	struct net_device	*indev;
	struct net_device	*outdev;
	int			(*okfn)(struct sk_buff *);

	/* extra space to store route keys */
};

#define nf_queue_entry_reroute(x) ((void *)x + sizeof(struct nf_queue_entry))

/* Packet queuing */
struct nf_queue_handler {
	int			(*outfn)(struct nf_queue_entry *entry,
					 unsigned int queuenum);
};
<<<<<<< HEAD
void nf_register_queue_handler(const struct nf_queue_handler *qh);
void nf_unregister_queue_handler(void);
extern void nf_reinject(struct nf_queue_entry *entry, unsigned int verdict);
=======

void nf_register_queue_handler(const struct nf_queue_handler *qh);
void nf_unregister_queue_handler(void);
extern void nf_reinject(struct nf_queue_entry *entry, unsigned int verdict);

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
bool nf_queue_entry_get_refs(struct nf_queue_entry *entry);
void nf_queue_entry_release_refs(struct nf_queue_entry *entry);

#endif /* _NF_QUEUE_H */
