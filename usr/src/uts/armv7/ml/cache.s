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
 * Copyright 2013 Joyent, Inc.  All rights reserved.
 */

	.file	"cache.s"

/* XXXARM: rework cache/tlb maintenance functions to handle ARMv7 */

/*
 * Cache and memory barrier operations
 */

#include <sys/asm_linkage.h>

#if defined(lint) || defined(__lint)

void
membar_sync(void)
{}

void
membar_enter(void)
{}

void
membar_exit(void)
{}

void
membar_producer(void)
{}

void
membar_consumer(void)
{}

void
instr_sbarrier(void)
{}

void
data_sbarrier(void)
{}

#else	/* __lint */

	/*
	 * NOTE: membar_enter, membar_exit, membar_producer, and
	 * membar_consumer are identical routines.  We define them
	 * separately, instead of using ALTENTRY definitions to alias
	 * them together, so that DTrace and debuggers will see a unique
	 * address for them, allowing more accurate tracing.
	 */
	ENTRY(membar_enter)
	ALTENTRY(membar_sync)
	dmb
	bx lr
	SET_SIZE(membar_sync)
	SET_SIZE(membar_enter)

	ENTRY(membar_exit)
	dmb
	bx lr
	SET_SIZE(membar_exit)

	ENTRY(membar_producer)
	dmb
	bx lr
	SET_SIZE(membar_producer)

	ENTRY(membar_consumer)
	dmb
	bx lr
	SET_SIZE(membar_consumer)

	ENTRY(instr_sbarrier)
	isb
	bx lr
	SET_SIZE(membar_consumer)

	ENTRY(data_sbarrier)
	isb
	bx lr
	SET_SIZE(data_sbarrier)

#endif	/* __lint */

#if defined(lint) || defined(__lint)

/*
 * The ARM architecture uses a modified Harvard Architecture which means that we
 * get the joys of fixing up this mess. Primarily this means that when we update
 * data, it gets written to do the data cache. That needs to be flushed to main
 * memory and then the instruction cache needs to be invalidated. This is
 * particularly important for things like krtld and DTrace. While the data cache
 * does write itself out over time, we cannot rely on it having written itself
 * out to the state that we care about by the time that we'd like it to. As
 * such, we need to ensure that it's been flushed out ourselves. This also means
 * that we could accidentally flush a region of the icache that's already
 * updated itself, but that's just what we have to do to keep Von Neumann's
 * spirt and great gift alive.
 *
 * The controllers for the caches have a few different options for invalidation.
 * One may:
 *
 *   o Invalidate or flush the entire cache
 *   o Invalidate or flush a cache line
 *   o Invalidate or flush a cache range
 *
 * We opt to take the third option here for the general case of making sure that
 * text has been synchronized. While the data cache allows us to both invalidate
 * and flush the cache line, we don't currently have a need to do the
 * invalidation.
 *
 * Note that all of these operations should be aligned on an 8-byte boundary.
 * The instructions actually only end up using bits [31:5] of an address.
 * Callers are required to ensure that this is the case.
 */

void
armv7_icache_disable(void)
{}

void
armv7_icache_enable(void)
{}

void
armv7_dcache_disable(void)
{}

void
armv7_dcache_enable(void)
{}

void
armv7_icache_inval(void)
{}

void
armv7_dcache_inval(void)
{}

void
armv7_dcache_flush(void)
{}

void
armv7_text_flush_range(caddr_t start, size_t len)
{}

void
armv7_text_flush(void)
{}

#else	/* __lint */

	ENTRY(armv7_icache_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x1000
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(armv7_icache_enable)

	ENTRY(armv7_dcache_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x4
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(armv7_dcache_enable)

	ENTRY(armv7_icache_disable)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x1000
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(armv7_icache_disable)

	ENTRY(armv7_dcache_disable)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x4
	mcr	p15, 0, r0, c1, c0, 0
	bx	lr
	SET_SIZE(armv7_dcache_disable)

	ENTRY(armv7_icache_inval)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Invalidate i-cache
	isb
	bx	lr
	SET_SIZE(armv7_icache_inval)

	/*
	 * The ARMv7 cache invalidation functions are very similar apart
	 * from the operation used and to what level, so we use a common
	 * macro for all.
	 *
	 * Valid options:
	 *
	 *  op: c6 (invalidate), c10 (clean), c14 (inv+clean aka flush)
	 *  all: 0 (PoU aka L1), 1 (PoC, default)
	 */
	.macro	dcache_operation op:req all=1
	mrc	p15, 1, r0, c0, c0, 1		@ r0 = CLIDR
	.if	\all
	tst	r0, #0x07000000			@ Check LoC
	.else
	tst	r0, #0x38000000			@ Check LoU
	.endif
	beq	4f				@ No cache levels
	mov	r3, #0				@ Start with CL1
1:	mcr	p15, 2, r3, c0, c0, 0		@ put level (r3) in CSSELR
	isb					@ sync to CSSIDR
	mrc	p15, 1, r0, c0, c0, 0		@ r0 = CSSIDR
	ubfx	ip, r0, #0, #3			@ ip = LineSize
	add	ip, ip, #4			@ ip += Level offset
	ubfx	r2, r0, #13, #15		@ r2 = NumSets - 1
	lsl	r2, r2, ip			@ r2 = shift by log2(LineSize)
	orr	r3, r3, r2			@ r3 |= Set
	mov	r1, #1
	lsl	r1, r1, ip			@ r1 = Set Decr
	ubfx	ip, r0, #3, #10			@ ip = NumWays
	clz	r2, ip				@ r2 = #bits to Way MSB
	lsl	ip, ip, r2			@ ip = shifted into position
	mov	r0, #1
	lsl	r2, r0, r2			@ r2 = Way Decr
	mov	r0, r3				@ r0 = Sets/Level
	orr	r3, r3, ip			@ r3 |= Way
	bfc	r0, #0, #4			@ r0 = NumSets - 1
	sub	r2, r2, r0			@ r2 -= NumSets - 1
2:	mcr	p15, 0, r3, c7, \op, 2		@ op = inval/clean/clean+inv
	cmp	r3, #15
	bls	3f
	ubfx	r0, r3, #4, #18
	cmp	r0, #0
	subne	r3, r3, r1			@ if (set) set--
	subeq	r3, r3, r2			@ if (!set) way--
	b	2b				@ keep going until done
3:	mrc	p15, 1, r0, c0, c0, 1		@ r0 = CLIDR
	.if	\all
	ubfx	ip, r0, #24, #3			@ ip = LoC
	.else
	ubfx	ip, r0, #27, #3			@ ip = LoU
	.endif
	add	r3, r3, #2			@ r3 = next cache level
	cmp	r3, ip, lsl #1			@ are we done to LoC/LoU?
	blt	1b				@ no, go around again
4:	mov	r0, #0				@ Default back to CL1
	mcr	p15, 2, r0, c0, c0, 0		@ Store cache level
	dsb	st
	isb
	.endm

	ENTRY(armv7_dcache_inval)
	dcache_operation c6			@ Invalidate d-cache
	bx	lr
	SET_SIZE(armv7_dcache_inval)

	ENTRY(armv7_dcache_flush)
	dcache_operation c14			@ Flush d-cache
	bx	lr
	SET_SIZE(armv7_dcache_flush)
	
	ENTRY(armv7_text_flush_range)
	mov	r2, #0
	mcr	p15, 2, r2, c0, c0, 0		@ Set CL1
	mrc	p15, 1, ip, c0, c0, 0		@ Read CCSIDR
	and	ip, ip, #7			@ LineSize[2:0] = log2(size) - 2
	mov	r2, #16
	lsl	ip, r2, ip			@ ip = cache line size
	sub	r2, ip, #1			@ r2 = linesize mask
	and	r3, r0, r2			@ r3 = offset of requested addr
	add	r1, r1, r3			@ Increase len to line size
	bic	r0, r0, r2			@ Align start to line address
1:	mcr	p15, 0, r0, c7, c5, 1		@ Inval i-cache range (ICIMVAU)
	mcr	p15, 0, r0, c7, c14, 1		@ Flush d-cache range (DCCIMVAC)
	add	r0, r0, ip			@ Go to next cache line
	subs	r1, r1, ip			@ Are we done?
	bhi	1b
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_text_flush_range)

	ENTRY(armv7_text_flush)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Invalidate i-cache
	mcr	p15, 0, r0, c7, c10, 4		@ Flush d-cache
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_text_flush)

#endif

#ifdef __lint

/*
 * Perform all of the operations necessary for tlb maintenance after an update
 * to the page tables.
 */
void
armv7_tlb_sync(void)
{}

#else	/* __lint */

	ENTRY(armv7_tlb_sync)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ Flush d-cache
	dsb
	mcr	p15, 0, r0, c8, c7, 0		@ invalidate tlb
	mcr	p15, 0, r0, c8, c5, 0		@ Invalidate I-cache + btc
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_tlb_sync)

#endif	/* __lint */
