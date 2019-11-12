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
 * Copyright (c) 2013 Joyent, Inc.  All rights reserved.
 */

#ifndef _SYS_PTE_H
#define	_SYS_PTE_H

/*
 * ARM page table descriptions and useful macros.
 */

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t armpte_t;

/*
 * ARM L1 Page Table Entry that points to an L2 table
 */
typedef struct arm_l1pt {
	uint32_t al_type:2;
	uint32_t al_ign0:1;
	uint32_t al_ns:1;
	uint32_t al_ign1:1;
	uint32_t al_domain:4;
	uint32_t al_imp:1;
	uint32_t al_ptaddr:22;
} arm_l1pt_t;

/*
 * ARM L1 Page Table Entry for a 1 MB section or 16 MB Super Section
 */
typedef struct arm_l1s {
	uint32_t al_type:2;
	uint32_t al_bbit:1;
	uint32_t al_cbit:1;
	uint32_t al_xn:1;
	uint32_t al_domain:4;
	uint32_t al_imp:1;
	uint32_t al_ap:2;
	uint32_t al_tex:3;
	uint32_t al_ap2:1;
	uint32_t al_sbit:1;
	uint32_t al_ngbit:1;
	uint32_t al_issuper:1;
	uint32_t al_nsbit:1;
	uint32_t al_addr:12;
} arm_l1s_t;

/*
 * ARM L2 Page Table Entry - 4KB page
 */
typedef struct arm_l2e {
	uint32_t ale_xn:1;
	uint32_t ale_ident:1;
	uint32_t ale_bbit:1;
	uint32_t ale_cbit:1;
	uint32_t ale_ap:2;
	uint32_t ale_tex:3;
	uint32_t ale_ap2:1;
	uint32_t ale_sbit:1;
	uint32_t ale_ngbit:1;
	uint32_t ale_addr:20;
} arm_l2e_t;

/*
 * ARM L2 Large Page entry - 64 KB
 */
typedef struct arm_l2le {
	uint32_t alle_ident:2;
	uint32_t alle_bbit:1;
	uint32_t alle_cbit:1;
	uint32_t alle_ign:3;
	uint32_t alle_ap:2;
	uint32_t alle_sbit:1;
	uint32_t alle_ngbit:1;
	uint32_t alle_tex:3;
	uint32_t alle_xn:1;
	uint32_t alle_addr:17;
} arm_l2le_t;

#define	ARMPT_L1_SIZE	(16 * 1024)
#define	ARMPT_L1_MASK	(0x3fff)

#define	ARMPT_L1_TYPE_INVALID	0x00
#define	ARMPT_L1_TYPE_L2PT	0x01
#define	ARMPT_L1_TYPE_SECT	0x02
#define	ARMPT_L1_TYPE_MASK	0x03

#define	ARMPT_VADDR_TO_L1E(vaddr)	(vaddr >> 20)
#define	ARMPT_ADDR_TO_L1PTADDR(addr)	(addr >> 10)
#define	ARMPT_PADDR_TO_L1SECT(addr)	(addr >> 20)
#define	ARMPT_L1E_ISVALID(entry)	\
	((entry & 0x3) != 0 && (entry & 0x3) != 0x3)

#define	ARMPT_L2_SIZE	(1024)
#define	ARMPT_L2_MASK	(0x3ff)
#define	ARMPT_L1PT_TO_L2_SHIFT	10

#define	ARMPT_L2_TYPE_INVALID	0x0
#define	ARMPT_L2_TYPE_LARGE	0x1
#define	ARMPT_L2_TYPE_MASK	0x3

#define	ARMPT_VADDR_TO_L2E(vaddr)	((vaddr & 0xff000) >> 12)
#define	ARMPT_PADDR_TO_L2ADDR(paddr)	(paddr >> 12)

#ifdef __cplusplus
}
#endif

#endif /* _SYS_PTE_H */
