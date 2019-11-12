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
 * Copyright 2019 Team Twist My ARM
 */

	.file	"aboot_ml.s"

/*
 * Varoius ASM routines for aboot.
 */

/*
 * Default TTBR values. For UP, this sets S, C, and Normal memory, Inner
 * Write-Back no Write-Allocate Cacheable. For MP it does the equivalent with
 * the MP extensions version which is split among two bits 0 and 6.
 */
#define	ABOOT_TTBR_UP	#0x1b
#define	ABOOT_TTBR_MP	#0x5b

#include <sys/asm_linkage.h>

	/*
	 * Turn off traps on unaligned access. Unaligned access is required in
	 * ARMv7.
	 */
	ENTRY(aboot_unaligned_enable)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x2		/* Toggle SCTLR.A */
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(aboot_unaligned_enable)

	/*
	 * Program the MMU. This includes making sure that the following are set
	 * up correclty:
	 *
	 *  - SCTLR.AFE is zero
	 *  - SCTLR.TRE is zero
	 *  - TTBCR is set to use TTBR0.
	 *  - Set DACR to allow client access to domain 0.
	 *  - Program the page table root in TBBR 0.
	 */
	ENTRY(aboot_mmu_program)
	mov	r1, #0
	mcr	p15, 0, r1, c2, c0, 2	/* Only use TBBR0 */

	mrc	p15, 0, r1, c1, c0, 0
	bic	r1, #0x20000000		/* Set AFE to zero */
	bic	r1, #0x10000000		/* Set TRE to zero */
	mcr	p15, 0, r1, c1, c0, 0

	mov	r1, #1
	mcr	p15, 0, r1, c3, c0, 0	/* Program DACR */

	mrc	p15, 0, r1, c0, c0, 5	/* Read MPIDR */
	cmp	r1, #0
	orrlt	r0, ABOOT_TTBR_MP	/* Use MP TTBR */
	orrge	r0, ABOOT_TTBR_UP	/* Use Base TTBR */
	mcr	p15,0, r0, c2, c0, 0	/* Write TTBR0 */

	bx	lr
	SET_SIZE(aboot_mmu_program)

	ENTRY(aboot_mmu_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x1		/* Toggle SCTLR.M (Enable MMU) */
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(aboot_mmu_enable)
