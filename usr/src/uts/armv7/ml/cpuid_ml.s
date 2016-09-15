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
 * Copyright 2014 Joyent, Inc.  All rights reserved.
 */

	.file	"cpuid.s"

/*
 * Read cpuid values from coprocessors
 */

#include <sys/asm_linkage.h>

#if defined(lint) || defined(__lint)

uint32_t
arm_cpuid_midr()
{}

uint32_t
arm_cpuid_pfr0()
{}

uint32_t
arm_cpuid_pfr1()
{}

uint32_t
arm_cpuid_dfr0()
{}

uint32_t
arm_cpuid_mmfr0()
{}

uint32_t
arm_cpuid_mmfr1()
{}

uint32_t
arm_cpuid_mmfr2()
{}

uint32_t
arm_cpuid_mmfr3()
{}

uint32_t
arm_cpuid_isar0()
{}

uint32_t
arm_cpuid_isar1()
{}

uint32_t
arm_cpuid_isar2()
{}

uint32_t
arm_cpuid_isar3()
{}

uint32_t
arm_cpuid_isar4()
{}

uint32_t
arm_cpuid_isar5()
{}

uint32_t
arm_cpuid_vfpidreg()
{}

uint32_t
arm_cpuid_mvfr0()
{}

uint32_t
arm_cpuid_mvfr1()
{}

#else	/* __lint */

	ENTRY(arm_cpuid_midr)
	mrc	p15, 0, r0, c0, c0, 0
	bx	lr
	SET_SIZE(arm_cpuid_midr)

	ENTRY(arm_cpuid_pfr0)
	mrc	p15, 0, r0, c0, c1, 0
	bx	lr
	SET_SIZE(arm_cpuid_pfr0)

	ENTRY(arm_cpuid_pfr1)
	mrc	p15, 0, r0, c0, c1, 1
	bx	lr
	SET_SIZE(arm_cpuid_pfr1)

	ENTRY(arm_cpuid_dfr0)
	mrc	p15, 0, r0, c0, c1, 2
	bx	lr
	SET_SIZE(arm_cpuid_dfr0)

	ENTRY(arm_cpuid_mmfr0)
	mrc	p15, 0, r0, c0, c1, 4
	bx	lr
	SET_SIZE(arm_cpuid_mmfr0)

	ENTRY(arm_cpuid_mmfr1)
	mrc	p15, 0, r0, c0, c1, 5
	bx	lr
	SET_SIZE(arm_cpuid_mmfr1)

	ENTRY(arm_cpuid_mmfr2)
	mrc	p15, 0, r0, c0, c1, 6
	bx	lr
	SET_SIZE(arm_cpuid_mmfr2)

	ENTRY(arm_cpuid_mmfr3)
	mrc	p15, 0, r0, c0, c1, 7
	bx	lr
	SET_SIZE(arm_cpuid_mmfr3)

	ENTRY(arm_cpuid_isar0)
	mrc	p15, 0, r0, c0, c2, 0
	bx	lr
	SET_SIZE(arm_cpuid_isar0)

	ENTRY(arm_cpuid_isar1)
	mrc	p15, 0, r0, c0, c2, 1
	bx	lr
	SET_SIZE(arm_cpuid_isar1)

	ENTRY(arm_cpuid_isar2)
	mrc	p15, 0, r0, c0, c2, 2
	bx	lr
	SET_SIZE(arm_cpuid_isar2)

	ENTRY(arm_cpuid_isar3)
	mrc	p15, 0, r0, c0, c2, 3
	bx	lr
	SET_SIZE(arm_cpuid_isar3)

	ENTRY(arm_cpuid_isar4)
	mrc	p15, 0, r0, c0, c2, 4
	bx	lr
	SET_SIZE(arm_cpuid_isar4)

	ENTRY(arm_cpuid_isar5)
	mrc	p15, 0, r0, c0, c2, 5
	bx	lr
	SET_SIZE(arm_cpuid_isar5)

	ENTRY(arm_cpuid_vfpidreg)
	vmrs	r0, FPSID
	bx	lr
	SET_SIZE(arm_cpuid_vfpidreg)

	ENTRY(arm_cpuid_mvfr0)
	vmrs	r0, MVFR0
	bx	lr
	SET_SIZE(arm_cpuid_mvfr0)

	ENTRY(arm_cpuid_mvfr1)
	vmrs	r0, MVFR1
	bx	lr
	SET_SIZE(arm_cpuid_mvfr1)
#endif /* __lint */

	ENTRY(arm_cpuid_clidr)
	mrc	p15, 1, r0, c0, c0, 1
	bx	lr
	SET_SIZE(arm_cpuid_clidr)

	ENTRY(arm_cpuid_ccsidr)
	lsl	r0, r0, #1
	cmp	r1, #0				/* icache == B_FALSE */
	orrne	r0, r0, #1
	mcr	p15, 2, r0, c0, c0, 0		/* write CSSELR */
	mrc	p15, 1, r0, c0, c0, 0		/* read selected CCSIDR */
	bx	lr
	SET_SIZE(arm_cpuid_ccsidr)
