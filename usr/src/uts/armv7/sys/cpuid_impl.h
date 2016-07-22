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
 * Copyright (c) 2014 Joyent, Inc.  All rights reserved.
 */

#ifndef _SYS_CPUID_IMPL_H
#define	_SYS_CPUID_IMPL_H

#include <sys/stdint.h>
#include <sys/arm_archext.h>
#include <sys/types.h>

/*
 * Routines to read ARM cpuid co-processors
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct arm_cpuid_cache {
	boolean_t acc_exists;
	boolean_t acc_unified;
	boolean_t acc_wt;
	boolean_t acc_wb;
	boolean_t acc_ra;
	boolean_t acc_wa;
	uint16_t acc_sets;
	uint8_t acc_linesz;
	uint16_t acc_assoc;

	boolean_t acc_rcolor;
	uint32_t acc_size;
} arm_cpuid_cache_t;

typedef struct arm_cpuid {
	uint32_t ac_ident;
	uint32_t ac_pfr[2];
	uint32_t ac_dfr;
	uint32_t ac_mmfr[4];
	uint32_t ac_isar[6];
	uint32_t ac_fpident;
	uint32_t ac_mvfr[2];
	uint32_t ac_clidr;

	/*
	 * ARM supports 7 levels of caches.  Each level can have separate
	 * I/D caches or a unified cache.  We keep track of all these as a
	 * two dimensional array.  First, we select if we're dealing with a
	 * I cache (B_TRUE) or a D/unified cache (B_FALSE), and then we
	 * index on the level.  Note that L1 caches are at index 0.
	 */
	uint32_t ac_ccsidr[2][7];
	arm_cpuid_cache_t ac_caches[2][7];
} arm_cpuid_t;

extern uint32_t arm_cpuid_midr();
extern uint32_t arm_cpuid_pfr0();
extern uint32_t arm_cpuid_pfr1();
extern uint32_t arm_cpuid_dfr0();
extern uint32_t arm_cpuid_mmfr0();
extern uint32_t arm_cpuid_mmfr1();
extern uint32_t arm_cpuid_mmfr2();
extern uint32_t arm_cpuid_mmfr3();
extern uint32_t arm_cpuid_isar0();
extern uint32_t arm_cpuid_isar1();
extern uint32_t arm_cpuid_isar2();
extern uint32_t arm_cpuid_isar3();
extern uint32_t arm_cpuid_isar4();
extern uint32_t arm_cpuid_isar5();

extern uint32_t arm_cpuid_vfpidreg();
extern uint32_t arm_cpuid_mvfr0();
extern uint32_t arm_cpuid_mvfr1();

extern uint32_t arm_cpuid_clidr();
extern uint32_t arm_cpuid_ccsidr(uint32_t level, boolean_t icache);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_CPUID_IMPL_H */
