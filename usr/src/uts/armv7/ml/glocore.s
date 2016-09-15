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
 * Copyright (c) 2015 Josef 'Jeff' Sipek <jeffpc@josefsipek.net>
 */

#include <sys/asm_linkage.h>
#include <sys/machparam.h>
#include <sys/cpu_asm.h>

#include "assym.h"

/*
 * Every story needs a beginning. This is ours.
 */

/*
 * Each of the different machines has its own locore.s to take care of getting
 * the machine specific setup done.  Just before jumping into fakebop the
 * first time, we call this machine specific code.
 */

/*
 * We are in a primordial world here. The loader is going to come along and
 * boot us at _start. As we've started the world, we also need to set up a
 * few things about us, for example our stack pointer. To help us out, it's
 * useful to remember what the loader set up for us:
 *
 * - unaligned access are allowed (A = 0, U = 1)
 * - virtual memory is enabled
 *   - we (unix) are mapped right were we want to be
 *   - a UART has been enabled & any memory mapped registers have been 1:1
 *     mapped
 *   - ATAGs have been updated to tell us what the mappings are
 * - I/D L1 caches have been enabled
 */

	/*
	 * External globals
	 */
	.globl	_locore_start
	.globl	mlsetup
	.globl	sysp
	.globl	bootops
	.globl	bootopsp
	.globl	t0

	.data
	.comm	t0stack, DEFAULTSTKSZ, 32
	.comm	t0, 4094, 32


/*
 * Recall that _start is the traditional entry point for an ELF binary.
 */
	ENTRY(_start)
	ldr	sp, =t0stack
	ldr	r4, =DEFAULTSTKSZ
	add	sp, r4
	bic	sp, sp, #0xff

	/*
	 * establish bogus stacks for exceptional CPU states, our exception
	 * code should never make use of these, and we want loud and violent
	 * failure should we accidentally try.
	 */
	cps	#(CPU_MODE_UND)
	mov	sp, #-1
	cps	#(CPU_MODE_ABT)
	mov	sp, #-1
	cps	#(CPU_MODE_FIQ)
	mov	sp, #-1
	cps	#(CPU_MODE_IRQ)
	mov	sp, #-1
	cps	#(CPU_MODE_SVC)

	/* Enable highvecs (moves the base of the exception vector) */
	mrc	p15, 0, r3, c1, c0, 0
	orr	r3, r3, #(1 << 13)
	mcr	p15, 0, r3, c1, c0, 0

	/* invoke machine specific setup */
	bl	_mach_start

	bl	_fakebop_start
	SET_SIZE(_start)


#if defined(__lint)

/* ARGSUSED */
void
_locore_start(struct boot_syscalls *sysp, struct bootops *bop)
{}

#else	/* __lint */

	/*
	 * We got here from _kobj_init() via exitto().  We have a few different
	 * tasks that we need to take care of before we hop into mlsetup and
	 * then main. We're never going back so we shouldn't feel compelled to
	 * preserve any registers.
	 *
	 *  o Save the boot syscalls and bootops for later
	 *  o Set up our stack to be the real stack of t0stack.
	 *  o Save t0 as curthread
	 *  o Set up a struct REGS for mlsetup
	 *  o Make sure that we're 8 byte aligned for the call
	 */

	ENTRY(_locore_start)


	/*
	 * We've been running in t0stack anyway, up to this point, but
	 * _locore_start represents what is in effect a fresh start in the
	 * real kernel -- We'll never return back through here.
	 *
	 * So reclaim those few bytes
	 */
	ldr	sp, =t0stack
	ldr	r4, =(DEFAULTSTKSZ - REGSIZE)
	add	sp, r4
	bic	sp, sp, #0xff

	/*
	 * Save flags and arguments for potential debugging
	 */
	str	r0, [sp, #REGOFF_R0]
	str	r1, [sp, #REGOFF_R1]
	str	r2, [sp, #REGOFF_R2]
	str	r3, [sp, #REGOFF_R3]
	mrs	r4, CPSR
	str	r4, [sp, #REGOFF_CPSR]

	/*
	 * Save back the bootops and boot_syscalls.
	 */
	ldr	r2, =sysp
	str	r0, [r2]
	ldr	r2, =bootops
	str	r1, [r2]
	ldr	r2, =bootopsp
	ldr	r2, [r2]
	str	r1, [r2]

	/*
	 * Set up our curthread pointer
	 */
	ldr	r0, =t0
	mcr	p15, 0, r0, c13, c0, 4

	/*
	 * mlsetup() takes the struct regs as an argument. main doesn't take
	 * any and should never return. Currently, we have an 8-byte aligned
	 * stack.  We want to push a zero frame pointer to terminate any
	 * stack walking, but that would cause us to end up with only a
	 * 4-byte aligned stack.  So, to keep things nice and correct, we
	 * push a zero value twice - it's similar to a typical function
	 * entry:
	 * 	push { r9, lr }
	 */
	mov	r9,#0
	push	{ r9 }		/* link register */
	push	{ r9 }		/* frame pointer */
	mov	r0, sp
	bl	mlsetup
	bl	main
	/* NOTREACHED */
	ldr	r0,=__return_from_main
	ldr	r0,[r0]
	bl 	panic
	SET_SIZE(_locore_start)

__return_from_main:
	.string "main() returned"
#endif	/* __lint */

	ENTRY(arm_reg_read)
	ldr r0, [r0]
	bx lr
	SET_SIZE(arm_reg_read)

	ENTRY(arm_reg_write)
	str r1, [r0]
	bx lr
	SET_SIZE(arm_reg_write)
