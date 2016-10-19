#ifndef __ASM_ARM_SYSTEM_MISC_H
#define __ASM_ARM_SYSTEM_MISC_H

#ifndef __ASSEMBLY__

#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/irqflags.h>
<<<<<<< HEAD
=======
#include <linux/reboot.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

extern void cpu_init(void);

void soft_restart(unsigned long);
<<<<<<< HEAD
extern void (*arm_pm_restart)(char str, const char *cmd);
=======
extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
extern void (*arm_pm_idle)(void);

#define UDBG_UNDEFINED	(1 << 0)
#define UDBG_SYSCALL	(1 << 1)
#define UDBG_BADABORT	(1 << 2)
#define UDBG_SEGV	(1 << 3)
#define UDBG_BUS	(1 << 4)

extern unsigned int user_debug;
<<<<<<< HEAD
=======
extern char* (*arch_read_hardware_id)(void);
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

#endif /* !__ASSEMBLY__ */

#endif /* __ASM_ARM_SYSTEM_MISC_H */
