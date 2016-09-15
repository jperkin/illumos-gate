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

/*
 * Copyright 2013 (c) Joyent, Inc. All rights reserved.
 * Copyright 2015 (c) Josef 'Jeff' Sipek <jeffpc@josefsipek.net>
 */

/*
 * Every story needs a beginning, this is the loader's.
 */

#include <sys/asm_linkage.h>
#include <sys/cpu_asm.h>

	.arch_extension virt
/*
 * We put _start into the .text.init section so we can more easily shove it
 * at the front of the .text.
 */
	.section .text.init
	.align	4
	.globl	_start
	.type	_start, %function
_start:
	mov	sp, #0x8000
	/*
	 * XXX manually fix up the tag start
	 */
	mov	r2, #0x100
	bl	fakeload_init
	SET_SIZE(_start)

#if defined(__lint)

/* ARGSUSED */
void
fakeload_unaligned_enable(void)
{}

#else 	/* __lint */

	ENTRY(fakeload_leave_hyp)
	/* Check if we are in Hypervisor mode */
	mrs	r0, cpsr
	and	r1, r0, #(CPU_MODE_MASK)
	teq	r1, #(CPU_MODE_HYP)
	bxne	lr

	/* Transition to Supervisor mode */
	mrs	r1, sp_hyp
	msr	sp_svc, r1
	bic	r0, r0, #(CPU_MODE_MASK)
	orr	r0, r0, #(CPU_MODE_SVC)
	msr	spsr_hyp, r0
	msr	elr_hyp, lr
	eret
	SET_SIZE(fakeload_leave_hyp)

	/*
	 * Fix up alignment by turning off A and by turning on U.
	 */
	ENTRY(fakeload_unaligned_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x400000	/* U = 1 */
	bic	r0, r0, #2	/* A = 0 */
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(fakeload_unaligned_enable);

#endif	/* __lint */

#if defined(__lint)

fakeload_pt_setup(uintptr_t ptroot)
{}

#else /* __lint */

	/*
	 * We need to set up the world for the first time. We'll do the
	 * following in order:
	 *
	 * o Set the TTBCR to always use TTBR0
	 * o Set domain 0 to manager mode
	 * o Program the Page table root
	 */
	ENTRY(fakeload_pt_setup)
	/* use TTBR0 only (should already be true) */
	mov	r1, #0
	mcr	p15, 0, r1, c2, c0, 2

	/* set domain 0 to manager mode */
	mov	r1, #3
	mcr	p15, 0, r1, c3, c0, 0

	/* set TTBR0 to page table root */
	orr	r0, r0, #0x18		/* Outer WB, no WA Cachable */
	orr	r0, r0, #0x2		/* Sharable */
	orr	r0, r0, #0x1		/* Inner Cachable */
	mcr	p15, 0, r0, c2, c0, 0
	bx	lr
	SET_SIZE(fakeload_pt_setup)

#endif /* __lint */

#if defined(__lint)

/* ARGSUSED */
void
fakeload_mmu_enable(void)
{}

#else	/* __lint */

	/*
	 * Enable the MMU (bit 0).
	 */
	ENTRY(fakeload_mmu_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x1		/* enable MMU */
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(fakeload_mmu_enable)
#endif	/* __lint */


	ENTRY(fakeload_exec)
	blx	r3
	/* We should never execute this. If we do we'll go back to a panic */
	bx	lr
	SET_SIZE(fakeload_exec)
