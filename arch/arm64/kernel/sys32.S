/*
 * Compat system call wrappers
 *
 * Copyright (C) 2012 ARM Ltd.
 * Authors: Will Deacon <will.deacon@arm.com>
 *	    Catalin Marinas <catalin.marinas@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/linkage.h>

#include <asm/assembler.h>
#include <asm/asm-offsets.h>

/*
 * System call wrappers for the AArch32 compatibility layer.
 */

compat_sys_sigreturn_wrapper:
	mov	x0, sp
	mov	x27, #0		// prevent syscall restart handling (why)
	b	compat_sys_sigreturn
ENDPROC(compat_sys_sigreturn_wrapper)

compat_sys_rt_sigreturn_wrapper:
	mov	x0, sp
	mov	x27, #0		// prevent syscall restart handling (why)
	b	compat_sys_rt_sigreturn
ENDPROC(compat_sys_rt_sigreturn_wrapper)

compat_sys_statfs64_wrapper:
	mov	w3, #84
	cmp	w1, #88
	csel	w1, w3, w1, eq
	b	compat_sys_statfs64
ENDPROC(compat_sys_statfs64_wrapper)

compat_sys_fstatfs64_wrapper:
	mov	w3, #84
	cmp	w1, #88
	csel	w1, w3, w1, eq
	b	compat_sys_fstatfs64
ENDPROC(compat_sys_fstatfs64_wrapper)

/*
 * Wrappers for AArch32 syscalls that either take 64-bit parameters
 * in registers or that take 32-bit parameters which require sign
 * extension.
 */
compat_sys_pread64_wrapper:
<<<<<<< HEAD
	orr	x3, x4, x5, lsl #32
=======
	regs_to_64	x3, x4, x5
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_pread64
ENDPROC(compat_sys_pread64_wrapper)

compat_sys_pwrite64_wrapper:
<<<<<<< HEAD
	orr	x3, x4, x5, lsl #32
=======
	regs_to_64	x3, x4, x5
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_pwrite64
ENDPROC(compat_sys_pwrite64_wrapper)

compat_sys_truncate64_wrapper:
<<<<<<< HEAD
	orr	x1, x2, x3, lsl #32
=======
	regs_to_64	x1, x2, x3
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_truncate
ENDPROC(compat_sys_truncate64_wrapper)

compat_sys_ftruncate64_wrapper:
<<<<<<< HEAD
	orr	x1, x2, x3, lsl #32
=======
	regs_to_64	x1, x2, x3
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_ftruncate
ENDPROC(compat_sys_ftruncate64_wrapper)

compat_sys_readahead_wrapper:
<<<<<<< HEAD
	orr	x1, x2, x3, lsl #32
=======
	regs_to_64	x1, x2, x3
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	mov	w2, w4
	b	sys_readahead
ENDPROC(compat_sys_readahead_wrapper)

compat_sys_fadvise64_64_wrapper:
	mov	w6, w1
<<<<<<< HEAD
	orr	x1, x2, x3, lsl #32
	orr	x2, x4, x5, lsl #32
=======
	regs_to_64	x1, x2, x3
	regs_to_64	x2, x4, x5
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	mov	w3, w6
	b	sys_fadvise64_64
ENDPROC(compat_sys_fadvise64_64_wrapper)

compat_sys_sync_file_range2_wrapper:
<<<<<<< HEAD
	orr	x2, x2, x3, lsl #32
	orr	x3, x4, x5, lsl #32
=======
	regs_to_64	x2, x2, x3
	regs_to_64	x3, x4, x5
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_sync_file_range2
ENDPROC(compat_sys_sync_file_range2_wrapper)

compat_sys_fallocate_wrapper:
<<<<<<< HEAD
	orr	x2, x2, x3, lsl #32
	orr	x3, x4, x5, lsl #32
=======
	regs_to_64	x2, x2, x3
	regs_to_64	x3, x4, x5
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
	b	sys_fallocate
ENDPROC(compat_sys_fallocate_wrapper)

#undef __SYSCALL
#define __SYSCALL(x, y)		.quad	y	// x

/*
 * The system calls table must be 4KB aligned.
 */
	.align	12
ENTRY(compat_sys_call_table)
#include <asm/unistd32.h>
