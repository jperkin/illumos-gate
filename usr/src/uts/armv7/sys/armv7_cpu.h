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

#ifndef _SYS_ARMV7_CPU_H
#define	_SYS_ARMV7_CPU_H

/*
 * Misc. functions that manipulate the ARMv7 CPU
 */

#include <sys/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void armv7_dcache_enable(void);
extern void armv7_dcache_disable(void);
extern void armv7_dcache_wbinval(void);
extern void armv7_dcache_dccisw(uint32_t);

extern void armv7_icache_enable(void);
extern void armv7_icache_disable(void);
extern void armv7_icache_inval(void);

extern void armv7_bpcache_enable(void);
extern void armv7_bpcache_disable(void);

extern uint32_t armv7_cache_clidr(void);
extern uint32_t armv7_cache_cssidr(uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_ARMV7_CPU_H */
