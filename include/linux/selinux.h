/*
 * SELinux services exported to the rest of the kernel.
 *
 * Author: James Morris <jmorris@redhat.com>
 *
 * Copyright (C) 2005 Red Hat, Inc., James Morris <jmorris@redhat.com>
 * Copyright (C) 2006 Trusted Computer Solutions, Inc. <dgoeddel@trustedcs.com>
 * Copyright (C) 2006 IBM Corporation, Timothy R. Chavez <tinytim@us.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation.
 */
#ifndef _LINUX_SELINUX_H
#define _LINUX_SELINUX_H

struct selinux_audit_rule;
struct audit_context;
struct kern_ipc_perm;

#ifdef CONFIG_SECURITY_SELINUX

/**
 * selinux_is_enabled - is SELinux enabled?
 */
bool selinux_is_enabled(void);
<<<<<<< HEAD
/**
 * selinux_is_enforcing - is SELinux Enforcing?
 */
bool selinux_is_enforcing(void);
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#else

static inline bool selinux_is_enabled(void)
{
	return false;
}
<<<<<<< HEAD

static inline bool selinux_is_enforcing(void)
{
	return false;
}
=======
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#endif	/* CONFIG_SECURITY_SELINUX */

#endif /* _LINUX_SELINUX_H */
