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
 * Copyright (c) 2015 Josef 'Jeff' Sipek <jeffpc@josefsipek.net>
 */

#include <sys/cpuid_impl.h>
#include <sys/param.h>
#include <sys/bootconf.h>
#include <vm/vm_dep.h>
#include <sys/armv7_bsmf.h>

/*
 * Handle classification and identification of ARM processors.
 *
 * Currently we do a single pass which reads in information and asserts that the
 * basic information which we receive here matches what we'd expect and are able
 * to do everything that we need with this ARM CPU.
 *
 * TODO We'll eventually do another pass to make sure that we properly determine
 * the feature set to expose to userland.
 */

static arm_cpuid_t cpuid_data0;

static uint32_t
extract(uint32_t line, uint32_t mask, uint32_t shift)
{
	return ((line & mask) >> shift);
}

static void
cpuid_fill_main(arm_cpuid_t *cpd)
{
	cpd->ac_pfr[0] = arm_cpuid_pfr0();
	cpd->ac_pfr[1] = arm_cpuid_pfr1();
	cpd->ac_dfr = arm_cpuid_dfr0();
	cpd->ac_mmfr[0] = arm_cpuid_mmfr0();
	cpd->ac_mmfr[1] = arm_cpuid_mmfr1();
	cpd->ac_mmfr[2] = arm_cpuid_mmfr2();
	cpd->ac_mmfr[3] = arm_cpuid_mmfr3();
	cpd->ac_isar[0] = arm_cpuid_isar0();
	cpd->ac_isar[1] = arm_cpuid_isar1();
	cpd->ac_isar[2] = arm_cpuid_isar2();
	cpd->ac_isar[3] = arm_cpuid_isar3();
	cpd->ac_isar[4] = arm_cpuid_isar4();
	cpd->ac_isar[5] = arm_cpuid_isar5();
}

static void
cpuid_fill_fpu(arm_cpuid_t *cpd)
{
	cpd->ac_mvfr[0] = arm_cpuid_mvfr0();
	cpd->ac_mvfr[1] = arm_cpuid_mvfr1();
}

#define	CCSIDR_WT		0x80000000
#define	CCSIDR_WB		0x40000000
#define	CCSIDR_RA		0x20000000
#define	CCSIDR_WA		0x10000000
#define	CCSIDR_NUMSETS_MASK	0x0fffe000
#define	CCSIDR_NUMSETS_SHIFT	13
#define	CCSIDR_ASSOC_MASK	0x00001ff8
#define	CCSIDR_ASSOC_SHIFT	3
#define	CCSIDR_LINESIZE_MASK	0x00000007
#define	CCSIDR_LINESIZE_SHIFT	0

static void
cpuid_fill_onecache(arm_cpuid_t *cpd, int level, boolean_t icache)
{
	arm_cpuid_cache_t *cache = &cpd->ac_caches[icache][level];
	uint32_t ccsidr;

	ccsidr = arm_cpuid_ccsidr(level, icache);
	cpd->ac_ccsidr[icache][level] = ccsidr;

	cache->acc_exists = B_TRUE;
	cache->acc_wt = (ccsidr & CCSIDR_WT) == CCSIDR_WT;
	cache->acc_wb = (ccsidr & CCSIDR_WB) == CCSIDR_WB;
	cache->acc_ra = (ccsidr & CCSIDR_RA) == CCSIDR_RA;
	cache->acc_wa = (ccsidr & CCSIDR_WA) == CCSIDR_WA;
	cache->acc_sets = extract(ccsidr, CCSIDR_NUMSETS_MASK,
	    CCSIDR_NUMSETS_SHIFT) + 1;
	cache->acc_assoc = extract(ccsidr, CCSIDR_ASSOC_MASK,
	    CCSIDR_ASSOC_SHIFT) + 1;
	cache->acc_linesz = sizeof (uint32_t) << (extract(ccsidr,
	    CCSIDR_LINESIZE_MASK, CCSIDR_LINESIZE_SHIFT) + 2);

/*
 * XXX?
#warning "set acc_size?"
 */
}

static void
cpuid_fill_caches(arm_cpuid_t *cpd)
{
	uint32_t erg, cwg;
	uint32_t l1ip;
	uint32_t ctr;
	uint32_t clidr;
	int level;

	clidr = arm_cpuid_clidr();
	cpd->ac_clidr = clidr;

	/* default all caches to not existing, and not unified */
	for (level = 0; level < 7; level++) {
		cpd->ac_caches[B_TRUE][level].acc_exists = B_FALSE;
		cpd->ac_caches[B_FALSE][level].acc_exists = B_FALSE;
		cpd->ac_caches[B_TRUE][level].acc_unified = B_FALSE;
		cpd->ac_caches[B_FALSE][level].acc_unified = B_FALSE;
	}

	/* retrieve cache info for each level */
	for (level = 0; level < 7; level++) {
		arm_cpuid_cache_t *icache = &cpd->ac_caches[B_TRUE][level];
		arm_cpuid_cache_t *dcache = &cpd->ac_caches[B_FALSE][level];
		uint32_t ctype = (cpd->ac_clidr >> (3 * level)) & 0x7;

		/* stop looking we find the first non-existent level */
		if (!ctype)
			break;

		switch (ctype) {
		case 1:
			cpuid_fill_onecache(cpd, level, B_TRUE);
			break;
		case 2:
			cpuid_fill_onecache(cpd, level, B_FALSE);
			break;
		case 3:
			cpuid_fill_onecache(cpd, level, B_TRUE);
			cpuid_fill_onecache(cpd, level, B_FALSE);
			break;
		case 4:
			cpuid_fill_onecache(cpd, level, B_FALSE);
			dcache->acc_unified = B_TRUE;
			break;
		default:
			bop_panic("unsupported cache type");
		}
	}

	/*
	 * We require L1-I/D & L2-D.  Unified caches are OK as well.
	 */
	if (!cpd->ac_caches[B_TRUE][0].acc_exists &&
	    (!cpd->ac_caches[B_FALSE][0].acc_exists ||
	    !cpd->ac_caches[B_FALSE][0].acc_unified))
		bop_panic("no L1 instructian cache detected");

	if (!cpd->ac_caches[B_FALSE][1].acc_exists)
		bop_panic("no L2 data cache detected");

	/*
	 * set globals with cache size info
	 */
	l2cache_sz = cpd->ac_caches[B_FALSE][1].acc_size;
	l2cache_linesz = cpd->ac_caches[B_FALSE][1].acc_linesz;
	l2cache_assoc = cpd->ac_caches[B_FALSE][1].acc_assoc;
}


/*
 * We need to do is go through and check for a few features that we know
 * we're going to need.
 */
static void
cpuid_verify(void)
{
	arm_cpuid_mem_vmsa_t vmsa;
	arm_cpuid_mem_barrier_t barrier;
	int sync, syncf;

	arm_cpuid_t *cpd = &cpuid_data0;

	/* v7 vmsa */
	vmsa = extract(cpd->ac_mmfr[0], ARM_CPUID_MMFR0_STATE0_MASK,
	    ARM_CPUID_MMFR0_STATE0_SHIFT);
	if (vmsa != ARM_CPUID_MEM_VMSA_V7) {
		bop_printf(NULL, "invalid vmsa setting, found 0x%x\n", vmsa);
		bop_panic("unsupported cpu");
	}

	/* check for ISB, DSB, etc. in cp15 */
	barrier = extract(cpd->ac_mmfr[2], ARM_CPUID_MMFR2_STATE5_MASK,
	    ARM_CPUID_MMFR2_STATE5_SHIFT);
	if (barrier != ARM_CPUID_MEM_BARRIER_INSTR) {
		bop_printf(NULL, "missing support for memory barrier "
		    "instructions\n");
		bop_panic("unsupported CPU");
	}

	/* synch prims */
	sync = extract(cpd->ac_isar[3], ARM_CPUID_ISAR3_STATE3_SHIFT,
	    ARM_CPUID_ISAR3_STATE3_SHIFT);
	syncf = extract(cpd->ac_isar[4], ARM_CPUID_ISAR4_STATE5_SHIFT,
	    ARM_CPUID_ISAR4_STATE5_SHIFT);
	if (sync != 0x2 && syncf != 0x0) {
		bop_printf(NULL, "unsupported synch primitives: sync,frac: "
		    "%x,%x\n", sync, syncf);
		bop_panic("unsupported CPU");
	}
}

static void
cpuid_valid_ident(uint32_t ident)
{
	arm_cpuid_ident_arch_t arch;

	/*
	 * We don't support anything older than ARMv7.
	 */
	arch = (ident & ARM_CPUID_IDENT_ARCH_MASK) >>
	    ARM_CPUID_IDENT_ARCH_SHIFT;
	if (arch != ARM_CPUID_IDENT_ARCH_CPUID) {
		bop_printf(NULL, "encountered unsupported CPU arch: 0x%x",
		    arch);
		bop_panic("unsupported CPU");
	}
}

static void
cpuid_valid_fpident(uint32_t ident)
{
	arm_cpuid_vfp_arch_t vfp;

	vfp = extract(ident, ARM_CPUID_VFP_ARCH_MASK, ARM_CPUID_VFP_ARCH_SHIFT);
	// XXX: _V3_V2BASE? _V3_NOBASE? _V3_V3BASE?
	if (vfp != ARM_CPUID_VFP_ARCH_V2) {
		bop_printf(NULL, "unsupported vfp version: %x\n", vfp);
		bop_panic("unsupported CPU");
	}

	if ((ident & ARM_CPUID_VFP_SW_MASK) != 0) {
		bop_printf(NULL, "encountered software-only vfp\n");
		bop_panic("unsuppored CPU");
	}
}
void
cpuid_setup(void)
{
	arm_cpuid_t *cpd = &cpuid_data0;

	cpd->ac_ident = arm_cpuid_midr();
	cpuid_valid_ident(cpd->ac_ident);
	cpuid_fill_main(cpd);

	cpd->ac_fpident = arm_cpuid_vfpidreg();
	cpuid_valid_fpident(cpd->ac_fpident);
	cpuid_fill_fpu(cpd);

	cpuid_fill_caches(cpd);

	cpuid_verify();
}
