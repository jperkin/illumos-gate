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

#if defined(_KERNEL)
#include <sys/types.h>
#include "reloc.h"
#else
#define	ELF_TARGET_ARM
#if defined(DO_RELOC_LIBLD)
#undef DO_RELOC_LIBLD
#define	DO_RELOC_LIBLD_ARM
#endif
#include <stdio.h>
#include "sgs.h"
#include <arm/machdep_arm.h>
#include "libld.h"
#include "reloc.h"
#include "conv.h"
#include "msg.h"
#endif

/*
 * We need to build this code differently when it is used for
 * cross linking:
 *	- Data alignment requirements can differ from those
 *		of the running system, so we can't access data
 *		in units larger than a byte
 *	- We have to include code to do byte swapping when the
 *		target and linker host use different byte ordering,
 *		but such code is a waste when running natively.
 */
#if !defined(DO_RELOC_LIBLD) || defined(__arm)
#define	DORELOC_NATIVE
#endif

/*
 * We have some relocations (R_ARM_CALL, say) that mask addend and value for
 * _replacement_, and some (R_ARM_PREL31) that merely have an odd number of
 * significant bits.
 *
 * We use re_mask for masking, and re_sigbits for significant bits, as appears
 * to make sense.
 *
 * Note thus that re_mask very very much implies that it's a pure replacement
 * of the bits matching the mask which occurs.
 *
 * XXXARM: Doing things the way we are means that we cannot check for, for eg,
 * overflow in R_ARM_CALL, because _we mask any bit that would indicate it_.
 * That means we're probably doing something wrong, and should probably be
 * taking the whole 32bits, check for (signed) under/overflow, and then only
 * using the low 24.
 *
 * XXXARM: There is a great chance that this is *ENTIRELY WRONG*, and happens
 * to work for my test cases because the compiler is being uncommonly kind.
 */
const Rel_entry reloc_table[R_ARM_NUM] = {
	{ 0, FLG_RE_NOTREL, 0, 0, 0 }, /* R_ARM_NONE */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PC24 */
	{ 0, FLG_RE_NOTREL, 4, 0, 0 }, /* R_ARM_ABS32 */
	{ 0, FLG_RE_PCREL,  4, 0, 0 }, /* R_ARM_REL32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_PC_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ABS16 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ABS12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_ABS5 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ABS8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_SBREL32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_CALL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_PC8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_BREL_ADJ */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_DESC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_THM_SWI8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_XPC25 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_THM_XPC22 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_DTPMOD32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_DTPOFF32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_TPOFF32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_COPY */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GLOB_DAT */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_JUMP_SLOT */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_RELATIVE */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOTOFF32 */
	{ 0, FLG_RE_SEGREL|FLG_RE_PCREL, 4, 0, 0 }, /* R_ARM_BASE_PREL */
	{ 0, FLG_RE_GOTADD, 4, 0, 0 }, /* R_ARM_GOT_BREL */
	/* Deprecated R_ARM_PLT32 */
	{ 0x00ffffff, FLG_RE_PLTREL|FLG_RE_PCREL, 4, 2, 0 },
	{ 0x00ffffff, FLG_RE_PCREL, 4, 2, 0 }, /* R_ARM_CALL */
	{ 0x00ffffff, FLG_RE_PCREL, 4, 2, 0 }, /* R_ARM_JUMP24 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_JUMP24 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_BASE_ABS */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_ALU_PCREL_7_0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_ALU_PCREL_15_8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_ALU_PCREL_23_15 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_LDR_SBREL_11_0_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_ALU_SBREL_19_12_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_ALU_SBREL_27_20_CK */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TARGET1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_SBREL31 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_V4BX */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TARGET2 */
	/* XXXARM: This is wrong, but also working.  Ouch. */
	{ 0x7fffffff, FLG_RE_NOTREL,  4, 0, 31 }, /* R_ARM_PREL31 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVW_ABS_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVT_ABS */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVW_PREL_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVT_PREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVW_ABS_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVT_ABS */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVW_PREL_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVT_PREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_R_ARM_THM_JUMP19 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_JUMP6 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_ALU_PREL_11_0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_PC12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ABS32_NOI */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_REL32_NOI */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_PC_G0_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_PC_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_PC_G1_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_PC_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_PC_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_PC_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_PC_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_PC_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_PC_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_PC_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_PC_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_PC_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_PC_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_SB_G0_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_SB_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_SB_G1_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_SB_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_ALU_SB_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_SB_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_SB_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDR_SB_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_SB_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_SB_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDRS_SB_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_SB_G0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_SB_G1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_LDC_SB_G2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVW_BREL_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVT_BREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_MOVW_BREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVW_BREL_NC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVT_BREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_MOVW_BREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_GOTDESC */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_CALL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_DESCSEQ */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_TLS_CALL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PLT32_ABS */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOT_ABS */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOT_PREL */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOT_BREL12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOTOFF12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_GOTRELAX */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_GNU_VTENTRY */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Deprecated R_ARM_GNU_VTINHERIT */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_JUMP11 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_JUMP8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_GD32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_LDM32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_LDO32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_IE32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_LE32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_LDO12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_LE12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_TLS_IE12GP */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_0 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_1 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_2 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_3 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_4 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_5 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_6 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_7 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_8 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_9 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_10 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_11 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_13 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_14 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_PRIVATE_15 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Obsolete R_ARM_ME_TOO */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_TLS_DESCSEQ16 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_TLS_DESCSEQ32 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_THM_GOT_BREL12 */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* Unallocated */
	{ 0, FLG_RE_NOTSUP, 0, 0, 0 }, /* R_ARM_IRELATIVE */
};


/*
 * Write a single relocated value to its reference location We assume we wish
 * to add the relocation amount, value, to the value of the adress already
 * present at the offset.
 *
 * Note that "T", relating to thumb interworking, is not actually supported.
 *
 * NAME			VALUE	FIELD	CALCULATION
 * R_ARM_NONE		0	none	none
 * R_ARM_ABS32		2	word32	(S + A) | T
 * R_ARM_REL32		3	word32	((S + A) | T) - P
 * R_ARM_BASE_PREL	25	word32	B(S) + A - P
 * R_ARM_GOT_BREL	26	word32	GOT(S) + A - GOT_ORG
 * R_ARM_PLT32		27	imm24	((S + A) | T) - P   XXX: Shifting
 * R_ARM_CALL		28	imm24	((S + A) | T) - P   XXX: Shifting
 * R_ARM_JUMP24		29	imm24	((S + A) | T) - P   XXX: Shifting
 * R_ARM_PREL31		42	imm31	((S + A) | T) - P   XXX: Shifting
 *
 * This is from Table 4-8, ELF for the ARM Architecture, ARM IHI 0044E,
 * current through ABI release 2.09, issued 30th November 2012.
 *
 * Relocation calculations:
 *
 * CALCULATION uses the following notation:
 * 	A	the addend used
 * 	B(S)	the address of S relative to the adddressing origin
 *		of the segment containing S.
 * 	T	1 if Thumb interworking, 0 if not.  Unsupported by our code.
 * 	P	the place being relocated
 * 	GOT(S)	the address of the GOT entry for S
 * 	GOT_ARG	the addressing origin of the global offset table
 *
 * The calculations in the CALCULATION column are assumed to have been
 * performed before calling this function except for the addition of the
 * addresses in the instructions.
 */
#if defined(_KERNEL)
#define	lml	0		/* Needed by arglist of REL_ERR_* macros */
int
do_reloc_krtld(uchar_t rtype, uchar_t *off, Xword *value, const char *sym,
    const char *file)
#elif defined(DO_RELOC_LIBLD)
int
do_reloc_ld(Rel_desc *rdesc, uchar_t *off, Xword *value,
    rel_desc_sname_func_t rel_desc_sname_func, const char *file,
    int bswap, void *lml)
#else
int
do_reloc_rtld(uchar_t rtype, uchar_t *off, Xword *value,
    const char *sym, const char *file, void *lml)
#endif
{
	const Rel_entry *rep;
#ifdef DO_RELOC_LIBLD
#define	sym (*rel_desc_sname_func)(rdesc)
	uchar_t 	rtype = rdesc->rel_rtype;
#endif
	Xword		base = 0, uvalue = 0;

	rep = &reloc_table[rtype];

	switch (rep->re_fsize) {
	case 4:
#if defined(DORELOC_NATIVE)
		base = *(Xword *)off;
#else
		if (bswap) {
			uchar_t *b_bytes = (uchar_t *)&base;
			UL_ASSIGN_BSWAP_WORD(b_bytes, off);
		} else {
			uchar_t *b_bytes = (uchar_t *)&base;
			UL_ASSIGN_WORD(b_bytes, off);
		}
#endif
		break;
	default:
		REL_ERR_UNSUPSZ(lml, file, sym, rtype, rep->re_fsize);
		return (0);
	}

	uvalue = *value;

	/*
	 * "If the relocation is pc-relative then compensation for the PC bias
	 * (the PC value is 8 bytes ahead of the executing instruction in ARM
	 * state and 4 bytes in Thumb state) must be encoded in the relocation
	 * by the object producer."  -- AEABI
	 */
	if (IS_PC_RELATIVE(rtype) && !IS_GOT_PC(rtype))
		uvalue -= 8;

	uvalue >>= rep->re_bshift;

	/*
	 * Masked values are masked both in and out
	 * for convenience, base is trunced.
	 *
	 * XXXARM: This of course means that any masked relocation is a
	 * replacement not masked addition.  I sure _hope_ that's true...
	 */
	if (rep->re_mask != 0) {
		uvalue &= rep->re_mask;
		base &= ~rep->re_mask;
	}

	/* Write the result */
	switch (rep->re_fsize) {
	case 4:
#if defined(DORELOC_NATIVE)
		*(Xword *)off = (Xword)(base + uvalue);
#else
		if (bswap) {
			uchar_t *b_bytes = (uchar_t *)&base;
			base += uvalue;
			UL_ASSIGN_BSWAP_WORD(off, b_bytes);
		} else {
			uchar_t *b_bytes = (uchar_t *)&base;
			base += uvalue;
			UL_ASSIGN_WORD(off, b_bytes);
		}
#endif
		break;
	}


	return (1);
#ifdef DO_RELOC_LIBLD
#undef sym
#endif
}
