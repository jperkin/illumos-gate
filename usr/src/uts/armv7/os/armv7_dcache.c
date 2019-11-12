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
 * Copyright 2019 Team Twist my ARM
 */

/*
 * Implements logic to target parts of the dcache. Unfortunately there is no
 * single invalidate all of the dcache instruction that we can use. Instead we
 * basically have to walk the cache levels, determine the number of ways and
 * sets, and flush all of them.
 */

#include <sys/types.h>
#include <sys/armv7_cpu.h>
#include <sys/bitmap.h>

#define	ARMV7_CLIDR_NLEVELS	7
#define	ARMV7_CLIDR_LEVEL(clidr, level)	(((clidr) >> (level * 3)) & 0x7)

#define	ARMV7_CSSIDR_LINESIZE(x)	BITX(x, 2, 0)
#define	ARMV7_CSSIDR_ASSOC(x)		BITX(x, 12, 3)
#define	ARMV7_CSSIDR_NSETS(x)		BITX(x, 27, 13)

typedef enum armv_cache_type {
	ARMV7_CACHE_NONE = 0x0,
	ARMV7_CACHE_ICACHE = 0x1,
	ARMV7_CACHE_DCACHE = 0x2,
	ARMV7_CACHE_SEPARATE = 0x3,
	ARMV7_CACHE_UNIFIED = 0x4
} armv7_cache_type_t;

static uint32_t
armv7_cache_clz(uint32_t x)
{
	uint32_t ret = 0, i;

	for (i = 0; i < 32; i++) {
		if ((x & (1 << i)) == 0) {
			ret++;
		}
	}

	return (ret);
}

void
armv7_dcache_wbinval(void)
{
	uint32_t clidr, i;

	clidr = armv7_cache_clidr();
	for (i = 0; i < ARMV7_CLIDR_NLEVELS; i++) {
		uint32_t cssidr, line, assoc, nways, nsets, way, set;
		uint32_t wayshift, setshift;
		armv7_cache_type_t type = ARMV7_CLIDR_LEVEL(clidr, i);

		/*
		 * If we encounter a cache with no type, then that
		 * architecturally is the last cache. If we only have an icache
		 * at this level, then there is no servicing that is required.
		 */
		switch (type) {
		case ARMV7_CACHE_NONE:
			break;
		case ARMV7_CACHE_ICACHE:
			continue;
		default:
			break;
		}

		/*
		 * To construct the way/set combination we need to put the level
		 * starting at bit 1. All bits that represent the line size
		 * should be zero.  The set number follows the line size bits,
		 * which is why it's at line (which is a log2 value) + 4. The
		 * way shift is determined by taking the log2 of the raw
		 * associativity value, rounded up.
		 */
		cssidr = armv7_cache_cssidr(i);
		line = ARMV7_CSSIDR_LINESIZE(cssidr);
		setshift = line + 4;
		nsets = ARMV7_CSSIDR_NSETS(cssidr) + 1;
		assoc = ARMV7_CSSIDR_ASSOC(cssidr);
		nways = assoc + 1;
		wayshift = armv7_cache_clz(assoc);

		for (way = 0; way < nways; way++) {
			for (set = 0; set < nsets; set++) {
				uint32_t ws;

				ws = i << 1;
				ws |= set << setshift;
				ws |= way << wayshift;
				armv7_dcache_dccisw(ws);
			}
		}
	}
}
