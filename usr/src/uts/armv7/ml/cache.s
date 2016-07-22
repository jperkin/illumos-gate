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
	SET_SIZE(armv7_icache_enable)

	ENTRY(armv7_dcache_enable)
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, #0x4
	mcr	p15, 0, r0, c1, c0, 0
	SET_SIZE(armv7_dcache_enable)

	ENTRY(armv7_icache_disable)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x1000
	mcr	p15, 0, r0, c1, c0, 0
	SET_SIZE(armv7_icache_disable)

	ENTRY(armv7_dcache_disable)
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x4
	mcr	p15, 0, r0, c1, c0, 0
	SET_SIZE(armv7_dcache_disable)

	ENTRY(armv7_icache_inval)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Invalidate i-cache
	bx	lr
	SET_SIZE(armv7_icache_inval)

	ENTRY(armv7_dcache_inval)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c6, 0		@ Invalidate d-cache
	dsb
	bx	lr
	SET_SIZE(armv7_dcache_inval)

	ENTRY(armv7_dcache_flush)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ Flush d-cache
	dsb
	bx	lr
	SET_SIZE(armv7_dcache_flush)
	
	ENTRY(armv7_text_flush_range)
	add	r1, r1, r0
	sub	r1, r1, r0
	mcrr	p15, 0, r1, r0, c5		@ Invalidate i-cache range
	mcrr	p15, 0, r1, r0, c12		@ Flush d-cache range
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
