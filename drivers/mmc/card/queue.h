#ifndef MMC_QUEUE_H
#define MMC_QUEUE_H

#define MMC_REQ_SPECIAL_MASK	(REQ_DISCARD | REQ_FLUSH)

struct request;
struct task_struct;

struct mmc_blk_request {
<<<<<<< HEAD
	struct mmc_request	mrq_que;
	struct mmc_request	mrq;
	struct mmc_command	sbc;
	struct mmc_command	que;
=======
	struct mmc_request	mrq;
	struct mmc_command	sbc;
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	struct mmc_command	cmd;
	struct mmc_command	stop;
	struct mmc_data		data;
};

enum mmc_packed_type {
	MMC_PACKED_NONE = 0,
	MMC_PACKED_WRITE,
};

#define mmc_packed_cmd(type)	((type) != MMC_PACKED_NONE)
#define mmc_packed_wr(type)	((type) == MMC_PACKED_WRITE)

struct mmc_packed {
	struct list_head	list;
	u32			cmd_hdr[1024];
	unsigned int		blocks;
	u8			nr_entries;
	u8			retries;
	s16			idx_failure;
};

struct mmc_queue_req {
	struct request		*req;
	struct mmc_blk_request	brq;
	struct scatterlist	*sg;
	char			*bounce_buf;
	struct scatterlist	*bounce_sg;
	unsigned int		bounce_sg_len;
	struct mmc_async_req	mmc_active;
	enum mmc_packed_type	cmd_type;
	struct mmc_packed	*packed;
<<<<<<< HEAD
	atomic_t		index;
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
};

struct mmc_queue {
	struct mmc_card		*card;
	struct task_struct	*thread;
	struct semaphore	thread_sem;
<<<<<<< HEAD
	unsigned int		flags;
#define MMC_QUEUE_SUSPENDED	(1 << 0)
#define MMC_QUEUE_NEW_REQUEST	(1 << 1)
=======
	unsigned long		flags;
#define MMC_QUEUE_SUSPENDED		0
#define MMC_QUEUE_NEW_REQUEST		1
#define MMC_QUEUE_URGENT_REQUEST	2
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	int			(*issue_fn)(struct mmc_queue *, struct request *);
	void			*data;
	struct request_queue	*queue;
<<<<<<< HEAD
	struct mmc_queue_req	mqrq[EMMC_MAX_QUEUE_DEPTH];
	struct mmc_queue_req	*mqrq_cur;
	struct mmc_queue_req	*mqrq_prev;
};

#define IS_RT_CLASS_REQ(x)	\
	(IOPRIO_PRIO_CLASS(req_get_ioprio(x)) == IOPRIO_CLASS_RT)

=======
	struct mmc_queue_req	mqrq[2];
	struct mmc_queue_req	*mqrq_cur;
	struct mmc_queue_req	*mqrq_prev;
	bool			wr_packing_enabled;
	int			num_of_potential_packed_wr_reqs;
	int			num_wr_reqs_to_start_packing;
	bool			no_pack_for_random;
	int (*err_check_fn) (struct mmc_card *, struct mmc_async_req *);
	void (*packed_test_fn) (struct request_queue *, struct mmc_queue_req *);
};

>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
extern int mmc_init_queue(struct mmc_queue *, struct mmc_card *, spinlock_t *,
			  const char *);
extern void mmc_cleanup_queue(struct mmc_queue *);
extern int mmc_queue_suspend(struct mmc_queue *, int);
extern void mmc_queue_resume(struct mmc_queue *);

extern unsigned int mmc_queue_map_sg(struct mmc_queue *,
				     struct mmc_queue_req *);
extern void mmc_queue_bounce_pre(struct mmc_queue_req *);
extern void mmc_queue_bounce_post(struct mmc_queue_req *);

extern int mmc_packed_init(struct mmc_queue *, struct mmc_card *);
extern void mmc_packed_clean(struct mmc_queue *);

<<<<<<< HEAD
extern void mmc_wait_cmdq_empty(struct mmc_host *);
=======
extern void print_mmc_packing_stats(struct mmc_card *card);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#endif
