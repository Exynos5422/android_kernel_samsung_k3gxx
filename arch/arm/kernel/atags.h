#ifdef CONFIG_ATAGS_PROC
extern void save_atags(struct tag *tags);
#else
static inline void save_atags(struct tag *tags) { }
#endif

void convert_to_tag_list(struct tag *tags);

#ifdef CONFIG_ATAGS
<<<<<<< HEAD
struct machine_desc *setup_machine_tags(phys_addr_t __atags_pointer, unsigned int machine_nr);
#else
static inline struct machine_desc *
=======
const struct machine_desc *setup_machine_tags(phys_addr_t __atags_pointer,
	unsigned int machine_nr);
#else
static inline const struct machine_desc *
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
setup_machine_tags(phys_addr_t __atags_pointer, unsigned int machine_nr)
{
	early_print("no ATAGS support: can't continue\n");
	while (true);
	unreachable();
}
#endif
