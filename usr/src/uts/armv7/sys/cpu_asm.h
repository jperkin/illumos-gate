/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

#ifndef _CPU_ASM_H
#define	_CPU_ASM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The ARM CPU has various states, mostly entered on exception
 */
#define	CPU_MODE_USR	0x10	/* User mode */
#define	CPU_MODE_FIQ	0x11	/* Fast interrupt */
#define	CPU_MODE_IRQ	0x12	/* Interrupt */
#define	CPU_MODE_SVC	0x13	/* Supervisor */
#define	CPU_MODE_MON	0x16	/* Monitor (security extension) */
#define	CPU_MODE_ABT	0x17	/* Abort (memory/insn access or breakpoint) */
#define	CPU_MODE_HYP	0x1a	/* Hypervisor */
#define	CPU_MODE_UND	0x1b	/* Undefined instruction */
#define	CPU_MODE_SYS	0x1f	/* System */
#define	CPU_MODE_MASK	0x1f

/*
 * ARM Exceptions (by their index in the exception vector)
 */
#define	ARM_EXCPT_RESET		0
#define	ARM_EXCPT_UNDINS	1
#define	ARM_EXCPT_SVC		2
#define	ARM_EXCPT_PREFETCH	3
#define	ARM_EXCPT_DATA		4
#define	ARM_EXCPT_RESERVED	5
#define	ARM_EXCPT_IRQ		6
#define	ARM_EXCPT_FIQ		7
#define	ARM_EXCPT_NUM		8

#ifdef __cplusplus
}
#endif

#endif /* _CPU_ASM_H */
