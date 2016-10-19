/*
 *  linux/arch/arm/mm/extable.c
 */
#include <linux/module.h>
#include <linux/uaccess.h>

int fixup_exception(struct pt_regs *regs)
{
	const struct exception_table_entry *fixup;

	fixup = search_exception_tables(instruction_pointer(regs));
<<<<<<< HEAD
	if (fixup)
		regs->ARM_pc = fixup->fixup;
=======
	if (fixup) {
		regs->ARM_pc = fixup->fixup;
#ifdef CONFIG_THUMB2_KERNEL
		/* Clear the IT state to avoid nasty surprises in the fixup */
		regs->ARM_cpsr &= ~PSR_IT_MASK;
#endif
	}
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

	return fixup != NULL;
}
