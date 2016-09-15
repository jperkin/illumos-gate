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

#ifndef _SYS_ARM_ARCHEXT_H
#define	_SYS_ARM_ARCHEXT_H

/*
 * Extensions to the ARM architecture as defined by the cpuid coprocessor
 * registers on ARM.
 *
 * ARM breaks registers into different states and then follows that with
 * enumerations to define the values of that state. This header follows in a
 * similar style where we define how to access those values and then
 * enumerations that describe those values
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Ident fields and masks */
#define	ARM_CPUID_IDENT_ARCH_MASK	0x000f0000
#define	ARM_CPUID_IDENT_ARCH_SHIFT	16
typedef	enum {
	ARM_CPUID_IDENT_ARCH_V4 = 0x1,
	ARM_CPUID_IDENT_ARCH_V4T2 = 0x2,
	ARM_CPUID_IDENT_ARCH_V5 = 0x3,
	ARM_CPUID_IDENT_ARCH_V5T = 0x4,
	ARM_CPUID_IDENT_ARCH_V5TE = 0x5,
	ARM_CPUID_IDENT_ARCH_V5TEJ = 0x6,
	ARM_CPUID_IDENT_ARCH_V6 = 0x7,
	ARM_CPUID_IDENT_ARCH_CPUID = 0xf
} arm_cpuid_ident_arch_t;

/* State 0 of the PFR describes ARM ISA support */
#define	ARM_CPUID_PFR0_STATE0_MASK	0x0000000f
#define	ARM_CPUID_PFR0_STATE0_SHIFT	0
typedef enum {
	ARM_CPUID_ISA_ARM_NONE = 0x0,
	ARM_CPUID_ISA_ARM_SUP = 0x1
} arm_cpuid_isa_arm_t;

/* State 1 of PFR0 describes Thumb support */
#define	ARM_CPUID_PFR0_STATE1_MASK	0x000000f0
#define	ARM_CPUID_PFR0_STATE1_SHIFT	4
typedef enum {
	ARM_CPUID_ISA_THUMB_NONE = 0x0,
	ARM_CPUID_ISA_THUMB_V1 = 0x1,
	ARM_CPUID_ISA_THUMB_V2 = 0x3
} arm_cpuid_isa_thumb_sup_t;

/* State2 of PFR0 describes Jazelle suport */
#define	ARM_CPUID_PFR0_STATE2_MASK	0x00000f00
#define	ARM_CPUID_PFR0_STATE2_SHIFT	8
typedef enum {
	ARM_CPUID_ISA_JAZELLE_NONE = 0x0,
	ARM_CPUID_ISA_JAZELLE_SUP = 0x1,
	ARM_CPUID_ISA_JAZELLE_EXT = 0x2
} arm_cpuid_isa_jazelle_sup_t;

/* State3 of PFR0 describes ThumbEE support */
#define	ARM_CPUID_PFR0_STATE3_MASK	0x0000f000
#define	ARM_CPUID_PFR0_STATE3_SHIFT	12
typedef enum {
	ARM_CPUID_ISA_THUMBEE_NONE = 0x0,
	ARM_CPUID_ISA_THUMBEE_SUP = 0x1
} arm_cpuid_isa_thumbee_sup_t;

/* State 0 of PFR1 describes the programmers model */
#define	ARM_CPUID_PFR1_STATE0_MASK	0x0000000f
#define	ARM_CPUID_PFR1_STATE0_SHIFT	0
typedef enum {
	ARM_CPUID_PMODEL_NONE = 0x0,
	ARM_CPUID_PMODEL_SUP = 0x1
} arm_cpuid_pmodel_t;

/* State 1 of PFR1 describes the security extensions */
#define	ARM_CPUID_PFR1_STATE1_MASK	0x000000f0
#define	ARM_CPUID_PFR1_STATE1_SHIFT	4
typedef enum {
	ARM_CPUID_ISA_SECEXT_NONE = 0x0,
	ARM_CPUID_ISA_SECEXT_SUP = 0x1,
	ARM_CPUID_ISA_SECEXT_EXT = 0x2
} arm_cpuid_isa_secext_sup_t;

/* State 2 of PFR1 is about microcontrollers programming model */
#define	ARM_CPUID_PFR2_STATE1_MASK	0x00000f00
#define	ARM_CPUID_PFR2_STATE1_SHIFT	8
typedef enum {
	ARM_CPUID_UPMODEL_NONE = 0x0,
	ARM_CPUID_UPMODEL_SUP = 0x2
} arm_cpuid_upmodel_t;

/* Coprocessor debugging features */
#define	ARM_CPUID_DFR0_STATE0_MASK	0x0000000f
#define	ARM_CPUID_DFR0_STATE0_SHIFT	0
typedef enum {
	ARM_CPUID_DEBUG_COPROC_NONE = 0x0,
	ARM_CPUID_DEBUG_COPROC_V6 = 0x2,
	ARM_CPUID_DEBUG_COPROC_V6_1 = 0x3,
	ARM_CPUID_DEBUG_COPROC_V7 = 0x4
} arm_cpuid_debug_coproc_t;

/* Secure coprocessor debugging features */
#define	ARM_CPUID_DFR0_STATE1_MASK	0x000000f0
#define	ARM_CPUID_DFR0_STATE1_SHIFT	4
typedef enum {
	ARM_CPUID_DEBUG_SCOPROC_NONE = 0x0,
	ARM_CPUID_DEBUG_SCOPROC_V6_1 = 0x3,
	ARM_CPUID_DEBUG_SCOPROC_V7 = 0x4
} arm_cpuid_debug_scoproc_t;

/* Memory mapped debug model */
#define	ARM_CPUID_DFR0_STATE2_MASK	0x00000f00
#define	ARM_CPUID_DFR0_STATE2_SHIFT	8
typedef enum {
	ARM_CPUID_DEBUG_MMAP_PROC_NONE = 0x0,
	ARM_CPUID_DEBUG_MMAP_PROC_V7 = 0x4
} arm_cpuid_debug_mmap_proc_t;

/* coprocessor trace model */
#define	ARM_CPUID_DFR0_STATE3_MASK	0x0000f000
#define	ARM_CPUID_DFR0_STATE3_SHIFT	12
typedef enum {
	ARM_CPUID_DEBUG_COPROC_TRACE_NONE = 0x0,
	ARM_CPUID_DEBUG_COPROC_TRACE_SUP = 0x1
} arm_cpuid_debug_coproc_trace_t;

/* mmap trace model */
#define	ARM_CPUID_DFR0_STATE4_MASK	0x000f0000
#define	ARM_CPUID_DFR0_STATE4_SHIFT	16
typedef enum {
	ARM_CPUID_DEBUG_MMAP_TRACE_NONE = 0x0,
	ARM_CPUID_DEBUG_MMAP_TRACE_SUP = 0x1
} arm_cpuid_debug_mmap_trace_t;

/* memory mapped microcontroller debuging */
#define	ARM_CPUID_DFR0_STATE5_MASK	0x00f00000
#define	ARM_CPUID_DFR0_STATE5_SHIFT	20
typedef enum {
	ARM_CPUID_DEBUG_MMAP_UPROC_NONE = 0x0,
	ARM_CPUID_DEBUG_MMAP_UPROC_SUP = 0x1
} arm_cpuid_debug_mmap_uproc_t;

/* AFR0 is reserved for chip makers */

/* VMSA Support */
#define	ARM_CPUID_MMFR0_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MMFR0_STATE0_SHIFT	0
typedef enum {
	ARM_CPUID_MEM_VMSA_NONE = 0x0,
	ARM_CPUID_MEM_VMSA_IMPL = 0x1,
	ARM_CPUID_MEM_VMSA_V6 = 0x2,
	ARM_CPUID_MEM_VMSA_V7 = 0x3,
	ARM_CPUID_MEM_VMSA_V7_PXN = 0x4,
	ARM_CPUID_MEM_VMSA_V7_EAE = 0x5,
} arm_cpuid_mem_vmsa_t;

/* PMSA Support */
#define	ARM_CPUID_MMFR0_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MMFR0_STATE1_SHIFT	4

/* Outermost shareability */
#define	ARM_CPUID_MMFR0_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MMFR0_STATE2_SHIFT	8

/* Shareability levels */
#define	ARM_CPUID_MMFR0_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MMFR0_STATE3_SHIFT	12

/* TCM support */
#define	ARM_CPUID_MMFR0_STATE4_MASK	0x000f0000
#define	ARM_CPUID_MMFR0_STATE4_SHIFT	16

/* Auxiliary registers */
#define	ARM_CPUID_MMFR0_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MMFR0_STATE5_SHIFT	20

/* FCSE support */
#define	ARM_CPUID_MMFR0_STATE6_MASK	0x0f000000
#define	ARM_CPUID_MMFR0_STATE6_SHIFT	24

/* Innermost shareability */
#define	ARM_CPUID_MMFR0_STATE7_MASK	0xf0000000
#define	ARM_CPUID_MMFR0_STATE7_SHIFT	28

/* L1 Harvard Cache VA */
#define	ARM_CPUID_MMFR1_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MMFR1_STATE0_SHIFT	0

/* L1 Unified Cache VA */
#define	ARM_CPUID_MMFR1_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MMFR1_STATE1_SHIFT	4

/* L1 Harvard Cache S/W */
#define	ARM_CPUID_MMFR1_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MMFR1_STATE2_SHIFT	8

/* L1 Unified Cache S/W */
#define	ARM_CPUID_MMFR1_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MMFR1_STATE3_SHIFT	12

/* L1 Harvard Cache */
#define	ARM_CPUID_MMFR1_STATE4_MASK	0x000f0000
#define	ARM_CPUID_MMFR1_STATE4_SHIFT	16

/* L1 Unified Cache */
#define	ARM_CPUID_MMFR1_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MMFR1_STATE5_SHIFT	20

/* L1 Cache test and clean */
#define	ARM_CPUID_MMFR1_STATE6_MASK	0x0f000000
#define	ARM_CPUID_MMFR1_STATE6_SHIFT	24

/* Branch predictor */
#define	ARM_CPUID_MMFR1_STATE7_MASK	0xf0000000
#define	ARM_CPUID_MMFR1_STATE7_SHIFT	28

/* L1 Harvard fg prefetch */
#define	ARM_CPUID_MMFR2_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MMFR2_STATE0_SHIFT	0

/* L1 Harvard bg prefetch */
#define	ARM_CPUID_MMFR2_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MMFR2_STATE1_SHIFT	4

/* L1 Harvard range */
#define	ARM_CPUID_MMFR2_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MMFR2_STATE2_SHIFT	8

/* Harvard tlb */
#define	ARM_CPUID_MMFR2_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MMFR2_STATE3_SHIFT	12

/* Unified tlb */
#define	ARM_CPUID_MMFR2_STATE4_MASK	0x000f0000
#define	ARM_CPUID_MMFR2_STATE4_SHIFT	16

/* Memory barrier */
#define	ARM_CPUID_MMFR2_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MMFR2_STATE5_SHIFT	20

/* WFI stall */
#define	ARM_CPUID_MMFR2_STATE6_MASK	0x0f000000
#define	ARM_CPUID_MMFR2_STATE6_SHIFT	24

/* HW access flag */
#define	ARM_CPUID_MMFR2_STATE7_MASK	0xf0000000
#define	ARM_CPUID_MMFR2_STATE7_SHIFT	28

/* Cache maintenance MVA */
#define	ARM_CPUID_MMFR3_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MMFR3_STATE0_SHIFT	0

/* Cache maintenance s/w */
#define	ARM_CPUID_MMFR3_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MMFR3_STATE1_SHIFT	4

/* branch predictor maintenance */
#define	ARM_CPUID_MMFR3_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MMFR3_STATE2_SHIFT	8

/* maintenance broadcast */
#define	ARM_CPUID_MMFR3_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MMFR3_STATE3_SHIFT	12

/* MMFR3 State 4 reserved */

/* coherent walk */
#define	ARM_CPUID_MMFR3_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MMFR3_STATE5_SHIFT	20

/* MMFR3 State 6 reserved */

/* Supersection support */
#define	ARM_CPUID_MMFR3_STATE7_MASK	0xf0000000
#define	ARM_CPUID_MMFR3_STATE7_SHIFT	28

/* swap instructions */
#define	ARM_CPUID_ISAR0_STATE0_MASK	0x0000000f
#define	ARM_CPUID_ISAR0_STATE0_SHIFT	0

/* bit count instructions */
#define	ARM_CPUID_ISAR0_STATE1_MASK	0x000000f0
#define	ARM_CPUID_ISAR0_STATE1_SHIFT	4

/* bit field instructions */
#define	ARM_CPUID_ISAR0_STATE2_MASK	0x00000f00
#define	ARM_CPUID_ISAR0_STATE2_SHIFT	8

/* compare and branch instructions */
#define	ARM_CPUID_ISAR0_STATE3_MASK	0x0000f000
#define	ARM_CPUID_ISAR0_STATE3_SHIFT	12

/* coprocessor instructions */
#define	ARM_CPUID_ISAR0_STATE4_MASK	0x000f0000
#define	ARM_CPUID_ISAR0_STATE4_SHIFT	16

/* debug instructions */
#define	ARM_CPUID_ISAR0_STATE5_MASK	0x00f00000
#define	ARM_CPUID_ISAR0_STATE5_SHIFT	20

/* divide instructions */
#define	ARM_CPUID_ISAR0_STATE6_MASK	0x0f000000
#define	ARM_CPUID_ISAR0_STATE6_SHIFT	24

/* endian instructions */
#define	ARM_CPUID_ISAR1_STATE0_MASK	0x0000000f
#define	ARM_CPUID_ISAR1_STATE0_SHIFT	0

/* exception instructions */
#define	ARM_CPUID_ISAR1_STATE1_MASK	0x000000f0
#define	ARM_CPUID_ISAR1_STATE1_SHIFT	4

/* exception A/R instructions */
#define	ARM_CPUID_ISAR1_STATE2_MASK	0x00000f00
#define	ARM_CPUID_ISAR1_STATE2_SHIFT	8

/* Extend instructions */
#define	ARM_CPUID_ISAR1_STATE3_MASK	0x0000f000
#define	ARM_CPUID_ISAR1_STATE3_SHIFT	12

/* If then instructions */
#define	ARM_CPUID_ISAR1_STATE4_MASK	0x000f0000
#define	ARM_CPUID_ISAR1_STATE4_SHIFT	16

/* immediate instructions */
#define	ARM_CPUID_ISAR1_STATE5_MASK	0x00f00000
#define	ARM_CPUID_ISAR1_STATE5_SHIFT	20

/* Interworking instructions */
#define	ARM_CPUID_ISAR1_STATE6_MASK	0x0f000000
#define	ARM_CPUID_ISAR1_STATE6_SHIFT	24

/* Jazelle instructoins */
#define	ARM_CPUID_ISAR1_STATE7_MASK	0xf0000000
#define	ARM_CPUID_ISAR1_STATE7_SHIFT	28

/* Load/store instructions */
#define	ARM_CPUID_ISAR2_STATE0_MASK	0x0000000f
#define	ARM_CPUID_ISAR2_STATE0_SHIFT	0

/* memory hint instructions */
#define	ARM_CPUID_ISAR2_STATE1_MASK	0x000000f0
#define	ARM_CPUID_ISAR2_STATE1_SHIFT	4

/* multi-access instructions are interruptible */
#define	ARM_CPUID_ISAR2_STATE2_MASK	0x00000f00
#define	ARM_CPUID_ISAR2_STATE2_SHIFT	8

/* Additional multiply instructions */
#define	ARM_CPUID_ISAR2_STATE3_MASK	0x0000f000
#define	ARM_CPUID_ISAR2_STATE3_SHIFT	12

/* signed multiply instructions */
#define	ARM_CPUID_ISAR2_STATE4_MASK	0x000f0000
#define	ARM_CPUID_ISAR2_STATE4_SHIFT	16

/* unsigned multiply instructions */
#define	ARM_CPUID_ISAR2_STATE5_MASK	0x00f00000
#define	ARM_CPUID_ISAR2_STATE5_SHIFT	20

/* v7 PSR manipulation */
#define	ARM_CPUID_ISAR2_STATE6_MASK	0x0f000000
#define	ARM_CPUID_ISAR2_STATE6_SHIFT	24

/* Reversal instructions */
#define	ARM_CPUID_ISAR2_STATE7_MASK	0xf0000000
#define	ARM_CPUID_ISAR2_STATE7_SHIFT	28

/* saturate instructions */
#define	ARM_CPUID_ISAR3_STATE0_MASK	0x0000000f
#define	ARM_CPUID_ISAR3_STATE0_SHIFT	0

/* SIMD instructions */
#define	ARM_CPUID_ISAR3_STATE1_MASK	0x000000f0
#define	ARM_CPUID_ISAR3_STATE1_SHIFT	4

/* SVC instructions */
#define	ARM_CPUID_ISAR3_STATE2_MASK	0x00000f00
#define	ARM_CPUID_ISAR3_STATE2_SHIFT	8

/* Sychronization primitives */
#define	ARM_CPUID_ISAR3_STATE3_MASK	0x0000f000
#define	ARM_CPUID_ISAR3_STATE3_SHIFT	12

/* Thumb table branch */
#define	ARM_CPUID_ISAR3_STATE4_MASK	0x000f0000
#define	ARM_CPUID_ISAR3_STATE4_SHIFT	16

/* Thumb copy instructions */
#define	ARM_CPUID_ISAR3_STATE5_MASK	0x00f00000
#define	ARM_CPUID_ISAR3_STATE5_SHIFT	20

/* NOP instruction */
#define	ARM_CPUID_ISAR3_STATE6_MASK	0x0f000000
#define	ARM_CPUID_ISAR3_STATE6_SHIFT	24

/* ThumbEE instructions */
#define	ARM_CPUID_ISAR3_STATE7_MASK	0xf0000000
#define	ARM_CPUID_ISAR3_STATE7_SHIFT	28

/* Unprivileged instructions */
#define	ARM_CPUID_ISAR4_STATE0_MASK	0x0000000f
#define	ARM_CPUID_ISAR4_STATE0_SHIFT	0

/* Support for instructions with shifts */
#define	ARM_CPUID_ISAR4_STATE1_MASK	0x000000f0
#define	ARM_CPUID_ISAR4_STATE1_SHIFT	4

/* Writeback instruction modes */
#define	ARM_CPUID_ISAR4_STATE2_MASK	0x00000f00
#define	ARM_CPUID_ISAR4_STATE2_SHIFT	8

/* SMC instruction support */
#define	ARM_CPUID_ISAR4_STATE3_MASK	0x0000f000
#define	ARM_CPUID_ISAR4_STATE3_SHIFT	12

/* Barrier instructions */
#define	ARM_CPUID_ISAR4_STATE4_MASK	0x000f0000
#define	ARM_CPUID_ISAR4_STATE4_SHIFT	16

/* Fractional synch primitives */
#define	ARM_CPUID_ISAR4_STATE5_MASK	0x00f00000
#define	ARM_CPUID_ISAR4_STATE5_SHIFT	20

/* M profile PSR modification */
#define	ARM_CPUID_ISAR4_STATE6_MASK	0x0f000000
#define	ARM_CPUID_ISAR4_STATE6_SHIFT	24

/* SWP and SWPB locking */
#define	ARM_CPUID_ISAR4_STATE7_MASK	0xf0000000
#define	ARM_CPUID_ISAR4_STATE7_SHIFT	28

/* ISAR5 is entirely reserved currently */

/* VFP identification flags and bits */
#define	ARM_CPUID_VFP_SW_MASK		0x00800000
#define	ARM_CPUID_VFP_ARCH_MASK		0x007f0000
#define	ARM_CPUID_VFP_ARCH_SHIFT	16
typedef enum {
	ARM_CPUID_VFP_ARCH_V1 = 0x0,
	ARM_CPUID_VFP_ARCH_V2 = 0x1,
	ARM_CPUID_VFP_ARCH_V3_V2BASE = 0x2,
	ARM_CPUID_VFP_ARCH_V3_NOBASE = 0x3,
	ARM_CPUID_VFP_ARCH_V3_V3BASE = 0x4
} arm_cpuid_vfp_arch_t;

/* Advanced SIMD bank */
#define	ARM_CPUID_MVFR0_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MVFR0_STATE0_SHIFT	0

/* single precision instructions */
#define	ARM_CPUID_MVFR0_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MVFR0_STATE1_SHIFT	4

/* double precision instructions */
#define	ARM_CPUID_MVFR0_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MVFR0_STATE2_SHIFT	8

/* VFP Exception trapping */
#define	ARM_CPUID_MVFR0_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MVFR0_STATE3_SHIFT	12

/* divide instructions */
#define	ARM_CPUID_MVFR0_STATE4_MASK	0x000f0000
#define	ARM_CPUID_MVFR0_STATE4_SHIFT	16

/* square root instructions */
#define	ARM_CPUID_MVFR0_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MVFR0_STATE5_SHIFT	20

/* short vectors */
#define	ARM_CPUID_MVFR0_STATE6_MASK	0x0f000000
#define	ARM_CPUID_MVFR0_STATE6_SHIFT	24

/* rounding modes */
#define	ARM_CPUID_MVFR0_STATE7_MASK	0xf0000000
#define	ARM_CPUID_MVFR0_STATE7_SHIFT	28

/* Flush to zero mode */
#define	ARM_CPUID_MVFR1_STATE0_MASK	0x0000000f
#define	ARM_CPUID_MVFR1_STATE0_SHIFT	0

/* default NaN mode */
#define	ARM_CPUID_MVFR1_STATE1_MASK	0x000000f0
#define	ARM_CPUID_MVFR1_STATE1_SHIFT	4

/* SIMD load/store instructions */
#define	ARM_CPUID_MVFR1_STATE2_MASK	0x00000f00
#define	ARM_CPUID_MVFR1_STATE2_SHIFT	8

/* SIMD integer instructions */
#define	ARM_CPUID_MVFR1_STATE3_MASK	0x0000f000
#define	ARM_CPUID_MVFR1_STATE3_SHIFT	12

/* SIMD single precision floating point */
#define	ARM_CPUID_MVFR1_STATE4_MASK	0x000f0000
#define	ARM_CPUID_MVFR1_STATE4_SHIFT	16

/* SIMD half precision floating point */
#define	ARM_CPUID_MVFR1_STATE5_MASK	0x00f00000
#define	ARM_CPUID_MVFR1_STATE5_SHIFT	20

/* VFP half precision floating point */
#define	ARM_CPUID_MVFR1_STATE6_MASK	0x0f000000
#define	ARM_CPUID_MVFR1_STATE6_SHIFT	24

#if	defined(_KERNEL)

extern void cpuid_setup(void);

#endif	/* _KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_ARM_ARCHEXT_H */
