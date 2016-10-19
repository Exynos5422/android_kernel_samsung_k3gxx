#ifndef _NF_DEFRAG_IPV6_H
#define _NF_DEFRAG_IPV6_H

extern void nf_defrag_ipv6_enable(void);

extern int nf_ct_frag6_init(void);
extern void nf_ct_frag6_cleanup(void);
extern struct sk_buff *nf_ct_frag6_gather(struct sk_buff *skb, u32 user);
<<<<<<< HEAD
extern void nf_ct_frag6_output(unsigned int hooknum, struct sk_buff *skb,
			       struct net_device *in,
			       struct net_device *out,
			       int (*okfn)(struct sk_buff *));
=======
extern void nf_ct_frag6_consume_orig(struct sk_buff *skb);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

struct inet_frags_ctl;

#endif /* _NF_DEFRAG_IPV6_H */
