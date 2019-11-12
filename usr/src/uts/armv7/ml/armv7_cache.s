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

	.file	"armv7_cache.s"

/*
 * XXX These should be conslidated elsewhere.
 */
#define	ARMV7_SCTLR_DCACHE	#0x4
#define	ARMV7_SCTLR_BPCACHE	#0x800
#define	ARMV7_SCTLR_ICACHE	#0x1000

/*
 * This implements functionality that enables, disables, cleans, and invalidates
 * caches.
 */

#include <sys/asm_linkage.h>

	ENTRY(armv7_dcache_enable)
	mcr	p15, 0, r0, c1, c0, 0
	orr	r0, ARMV7_SCTLR_DCACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_dcache_enable)

	ENTRY(armv7_dcache_disable)
	mcr	p15, 0, r0, c1, c0, 0
	bic	r0, ARMV7_SCTLR_DCACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_dcache_disable)

	ENTRY(armv7_bpcache_enable)
	mcr	p15, 0, r0, c1, c0, 0
	orr	r0, ARMV7_SCTLR_BPCACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_bpcache_enable)

	ENTRY(armv7_bpcache_disable)
	mcr	p15, 0, r0, c1, c0, 0
	bic	r0, ARMV7_SCTLR_BPCACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_bpcache_disable)

	ENTRY(armv7_icache_enable)
	mcr	p15, 0, r0, c1, c0, 0
	orr	r0, ARMV7_SCTLR_ICACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_icache_enable)

	ENTRY(armv7_icache_disable)
	mcr	p15, 0, r0, c1, c0, 0
	bic	r0, ARMV7_SCTLR_ICACHE
	mrc	p15, 0, r0, c1, c0, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_icache_disable)

	ENTRY(armv7_icache_inval)
	mcr	p15, 0, r0, c7, c5, 0
	dsb
	isb
	bx	lr
	SET_SIZE(armv7_icache_inval)

	ENTRY(armv7_cache_clidr)
	mrc	p15, 1, r0, c0, c0, 1
	bx	lr
	SET_SIZE(armv7_cache_clidr)

	ENTRY(armv7_cache_cssidr)
	lsl	r0, #1
	mcr 	p15, 2, r0, c0, c0, 0
	isb
	mrc	p15, 1, r0, c0, c0, 0
	bx	lr
	SET_SIZE(armv7_cache_cssidr)

	ENTRY(armv7_dcache_dccisw)
	mcr	p15, 0, r0, c7, c14, 2
	dsb
	bx	lr
	SET_SIZE(armv7_dcache_dccisw)	
