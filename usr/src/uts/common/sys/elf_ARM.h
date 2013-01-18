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
 * Copyright 2013, Richard Lowe
 */

#ifndef _SYS_ELF_ARM_H
#define	_SYS_ELF_ARM_H

#ifdef __cplusplus
extern "C" {
#endif

#define	R_ARM_NONE			0
#define	R_ARM_PC24			1
#define	R_ARM_ABS32			2
#define	R_ARM_REL32			3
#define	R_ARM_LDR_PC_G0			4
#define	R_ARM_ABS16			5
#define	R_ARM_ABS12			6
#define	R_ARM_THM_ABS5			7
#define	R_ARM_ABS8			8
#define	R_ARM_SBREL32			9
#define	R_ARM_THM_CALL			10
#define	R_ARM_THM_PC8			11
#define	R_ARM_BREL_ADJ			12
#define	R_ARM_TLS_DESC			13
#define	R_ARM_THM_SWI8			14	/* Obsolete */
#define	R_ARM_XPC25			15	/* Obsolete */
#define	R_ARM_THM_XPC22			16	/* Obsolete */
#define	R_ARM_TLS_DTPMOD32		17
#define	R_ARM_TLS_DTPOFF32		18
#define	R_ARM_TLS_TPOFF32		19
#define	R_ARM_COPY			20
#define	R_ARM_GLOB_DAT			21
#define	R_ARM_JUMP_SLOT			22
#define	R_ARM_RELATIVE			23
#define	R_ARM_GOTOFF32			24
#define	R_ARM_BASE_PREL			25
#define	R_ARM_GOT_BREL			26
#define	R_ARM_PLT32			27	/* Deprecated */
#define	R_ARM_CALL			28
#define	R_ARM_JUMP24			29
#define	R_ARM_THM_JUMP24		30
#define	R_ARM_BASE_ABS			31
#define	R_ARM_ALU_PCREL_7_0		32	/* Obsolete */
#define	R_ARM_ALU_PCREL_15_8		33	/* Obsolete */
#define	R_ARM_ALU_PCREL_23_15		34	/* Obsolete */
#define	R_ARM_LDR_SBREL_11_0_NC		35	/* Deprecated */
#define	R_ARM_ALU_SBREL_19_12_NC	36	/* Deprecated */
#define	R_ARM_ALU_SBREL_27_20_CK	37	/* Deprecated */
#define	R_ARM_TARGET1			38
#define	R_ARM_SBREL31			39	/* Deprecated */
#define	R_ARM_V4BX			40
#define	R_ARM_TARGET2			41
#define	R_ARM_PREL31			42
#define	R_ARM_MOVW_ABS_NC		43
#define	R_ARM_MOVT_ABS			44
#define	R_ARM_MOVW_PREL_NC		45
#define	R_ARM_MOVT_PREL			46
#define	R_ARM_THM_MOVW_ABS_NC		47
#define	R_ARM_THM_MOVT_ABS		48
#define	R_ARM_THM_MOVW_PREL_NC		49
#define	R_ARM_THM_MOVT_PREL		50
#define	R_ARM_THM_JUMP19		51
#define	R_ARM_THM_JUMP6			52
#define	R_ARM_THM_ALU_PREL_11_0		53
#define	R_ARM_THM_PC12			54
#define	R_ARM_ABS32_NOI			55
#define	R_ARM_REL32_NOI			56
#define	R_ARM_ALU_PC_G0_NC		57
#define	R_ARM_ALU_PC_G0			58
#define	R_ARM_ALU_PC_G1_NC		59
#define	R_ARM_ALU_PC_G1			60
#define	R_ARM_ALU_PC_G2			61
#define	R_ARM_LDR_PC_G1			62
#define	R_ARM_LDR_PC_G2			63
#define	R_ARM_LDRS_PC_G0		64
#define	R_ARM_LDRS_PC_G1		65
#define	R_ARM_LDRS_PC_G2		66
#define	R_ARM_LDC_PC_G0			67
#define	R_ARM_LDC_PC_G1			68
#define	R_ARM_LDC_PC_G2			69
#define	R_ARM_ALU_SB_G0_NC		70
#define	R_ARM_ALU_SB_G0			71
#define	R_ARM_ALU_SB_G1_NC		72
#define	R_ARM_ALU_SB_G1			73
#define	R_ARM_ALU_SB_G2			74
#define	R_ARM_LDR_SB_G0			75
#define	R_ARM_LDR_SB_G1			76
#define	R_ARM_LDR_SB_G2			77
#define	R_ARM_LDRS_SB_G0		78
#define	R_ARM_LDRS_SB_G1		79
#define	R_ARM_LDRS_SB_G2		80
#define	R_ARM_LDC_SB_G0			81
#define	R_ARM_LDC_SB_G1			82
#define	R_ARM_LDC_SB_G2			83
#define	R_ARM_MOVW_BREL_NC		84
#define	R_ARM_MOVT_BREL			85
#define	R_ARM_MOVW_BREL			86
#define	R_ARM_THM_MOVW_BREL_NC		87
#define	R_ARM_THM_MOVT_BREL		88
#define	R_ARM_THM_MOVW_BREL		89
#define	R_ARM_TLS_GOTDESC		90
#define	R_ARM_TLS_CALL			91
#define	R_ARM_TLS_DESCSEQ		92
#define	R_ARM_THM_TLS_CALL		93
#define	R_ARM_PLT32_ABS			94
#define	R_ARM_GOT_ABS			95
#define	R_ARM_GOT_PREL			96
#define	R_ARM_GOT_BREL12		97
#define	R_ARM_GOTOFF12			98
#define	R_ARM_GOTRELAX			99
#define	R_ARM_GNU_VTENTRY		100	/* Deprecated */
#define	R_ARM_GNU_VTINHERIT		101	/* Deprecated */
#define	R_ARM_THM_JUMP11		102
#define	R_ARM_THM_JUMP8			103
#define	R_ARM_TLS_GD32			104
#define	R_ARM_TLS_LDM32			105
#define	R_ARM_TLS_LDO32			106
#define	R_ARM_TLS_IE32			107
#define	R_ARM_TLS_LE32			108
#define	R_ARM_TLS_LDO12			109
#define	R_ARM_TLS_LE12			110
#define	R_ARM_TLS_IE12GP		111
#define	R_ARM_PRIVATE_0			112
#define	R_ARM_PRIVATE_1			113
#define	R_ARM_PRIVATE_2			114
#define	R_ARM_PRIVATE_3			115
#define	R_ARM_PRIVATE_4			116
#define	R_ARM_PRIVATE_5			117
#define	R_ARM_PRIVATE_6			118
#define	R_ARM_PRIVATE_7			119
#define	R_ARM_PRIVATE_8			120
#define	R_ARM_PRIVATE_9			121
#define	R_ARM_PRIVATE_10		122
#define	R_ARM_PRIVATE_11		123
#define	R_ARM_PRIVATE_12		124
#define	R_ARM_PRIVATE_13		125
#define	R_ARM_PRIVATE_14		126
#define	R_ARM_PRIVATE_15		127
#define	R_ARM_ME_TOO			128	/* Obsolete */
#define	R_ARM_THM_TLS_DESCSEQ16		129
#define	R_ARM_THM_TLS_DESCSEQ32		130
#define	R_ARM_THM_GOT_BREL12		131
/* 132-139 unallocated */
#define	R_ARM_IRELATIVE			140
/* 141-255 unallocated */

#define	R_ARM_NUM			141

#define	ELF_ARM_MAXPGSZ	0x08000

#define	EF_ARM_EABI_MASK	0xff000000 /* ABI version */
#define	EF_ARM_EABI_VER1	0x01000000
#define	EF_ARM_EABI_VER2	0x02000000
#define	EF_ARM_EABI_VER3	0x03000000
#define	EF_ARM_EABI_VER4	0x04000000
#define	EF_ARM_EABI_VER5	0x05000000

/*
 * The ARM ABI documents remove flags as they fall out of use, and reserve
 * them to themselves.  Several of these values, thus, may be reused in later
 * versions of the ABI.
 */
#define	EF_ARM_RELEXEC		0x00000001
#define	EF_ARM_HASENTRY		0x00000002
#define	EF_ARM_INTERWORK	0x00000004
#define	EF_ARM_APCS_26		0x00000008
#define	EF_ARM_APCS_FLOAT	0x00000010
#define	EF_ARM_PIC		0x00000020
#define	EF_ARM_ALIGN8		0x00000040
#define	EF_ARM_NEW_ABI		0x00000080
#define	EF_ARM_OLD_ABI		0x00000100
#define	EF_ARM_ABI_FLOAT_SOFT	0x00000200 /* software fp calling convention */
#define	EF_ARM_ABI_FLOAT_HARD	0x00000400 /* hardware fp calling convention */
#define	EF_ARM_MAVERICK_FLOAT	0x00000800
#define	EF_ARM_LE8		0x00400000
#define	EF_ARM_BE8		0x00800000 /* Contains BE8 code for v6 */
#define	EF_ARM_GCCMASK		0x00400FFF /* Legacy GCC mask for ABI v4 */

/* ARM-specific section types */
#define	SHT_ARM_EXIDX		0x70000001 /* Exception Index table */
#define	SHT_ARM_PREEMPTMAP	0x70000002 /* BPABI linking pre-emption map */
#define	SHT_ARM_ATTRIBUTES	0x70000003 /* Object compatibility attributes */
#define	SHT_ARM_DEBUGOVERLAY	0x70000004
#define	SHT_ARM_OVERLAYSECTION	0x70000005

/* ARM-specific program headers */
#define	PT_ARM_ARCHEXT		0x70000000
#define	PT_ARM_EXIDX		0x70000001
#define	PT_ARM_UNWIND		PT_ARM_EXIDX

/* ARM attributes */
#define	ARM_ATTR_VERSION	'A'
#define	ARM_TAG_FILE				1	/* uint32 */
#define	ARM_TAG_SECTION				2	/* uint32 */
#define	ARM_TAG_SYMBOL				3	/* uint32 */
#define	ARM_TAG_CPU_RAW_NAME			4	/* char* */
#define	ARM_TAG_CPU_NAME			5	/* char* */
#define	ARM_TAG_CPU_ARCH			6	/* uleb128 */
#define	ARM_TAG_CPU_ARCH_PROFILE		7	/* uleb128 */
#define	ARM_TAG_ARM_ISA_USE			8	/* uleb128 */
#define	ARM_TAG_THUMB_ISA_USE			9	/* uleb128 */
#define	ARM_TAG_FP_ARCH				10	/* uleb128 */
#define	ARM_TAG_WWMX_ARCH			11	/* uleb128 */
#define	ARM_TAG_ADVANCED_SIMD_ARCH		12	/* uleb128 */
#define	ARM_TAG_ABI_PCS_CONFIG			13	/* uleb128 */
#define	ARM_TAG_ABI_PCS_R9_USE			14	/* uleb128 */
#define	ARM_TAG_ABI_PCS_RW_DATA			15	/* uleb128 */
#define	ARM_TAG_ABI_PCS_RO_DATA			16	/* uleb128 */
#define	ARM_TAG_ABI_PCS_GOT_USE			17	/* uleb128 */
#define	ARM_TAG_ABI_PCS_WCHAR_T			18	/* uleb128 */
#define	ARM_TAG_ABI_FP_ROUNDING			19	/* uleb128 */
#define	ARM_TAG_ABI_FP_DENORMAL			20	/* uleb128 */
#define	ARM_TAG_ABI_FP_EXCEPTIONS		21	/* uleb128 */
#define	ARM_TAG_ABI_FP_USER_EXCEPTIONS		22	/* uleb128 */
#define	ARM_TAG_ABI_FP_NUMBER_MODEL		23	/* uleb128 */
#define	ARM_TAG_ABI_ALIGN_NEEDED		24	/* uleb128 */
#define	ARM_TAG_ABI_ALIGN_PRESERVED		25	/* uleb128 */
#define	ARM_TAG_ABI_ENUM_SIZE			26	/* uleb128 */
#define	ARM_TAG_ABI_HARDFP_USE			27	/* uleb128 */
#define	ARM_TAG_ABI_VFP_ARGS			28	/* uleb128 */
#define	ARM_TAG_ABI_WMMX_ARGS			29	/* uleb128 */
#define	ARM_TAG_ABI_OPTIMIZATION_GOALS		30	/* uleb128 */
#define	ARM_TAG_ABI_FP_OPTIMIZATION_GOALS	31	/* uleb128 */
#define	ARM_TAG_COMPATIBILITY			32	/* char* */
/*						33 unused */
#define	ARM_TAG_UNALIGNED_ACCESS		34	/* uleb128 */
/*						35 unused */
#define	ARM_TAG_FP_HP_EXTENSION			36	/* uleb128 */
/*						37 unused */
#define	ARM_TAG_ABI_16BIT_FORMAT		38	/* uleb128 */
/*						39-41 unused */
#define	ARM_TAG_MPEXTENSION_USE			42	/* uleb128 */
/*						43 unused */
#define	ARM_TAG_DIV_USE				44	/* uleb128 */
/*						45-63 unused */
#define	ARM_TAG_NODEFAULTS			64	/* uleb128 */
#define	ARM_TAG_ALSO_COMPATIBLE_WITH		65	/* char* */
#define	ARM_TAG_T2EE_USE			66	/* uleb128 */
#define	ARM_TAG_CONFORMANCE			67	/* char* */
#define	ARM_TAG_VIRTUALIZATION_USE		68	/* uleb128 */
#define	ARM_TAG_MPEXTENSION_USE_2		70	/* uleb128 (now #42) */

/*
 * There are consumers of this file that want to include elf defines for
 * all architectures.  This is a problem for the defines below, because
 * while they are architecture specific they have common names.  Hence to
 * prevent attempts to redefine these variables we'll check if any of
 * the other elf architecture header files have been included.  If
 * they have then we'll just stick with the existing definitions.
 */
#if !defined(_SYS_ELF_MACH_COMMON)
#define	_SYS_ELF_MACH_COMMON
#define	_SYS_ELF_MACH_ARM

#define	M_PLT_ENTSIZE	12	/* PLT entry size in bytes */
#define	M_PLT_INSSIZE	4	/* Size of each PLT insn */
#define	M_PLT_ALIGN	4	/* PLT is word aligned, since it's ARM code */

#define	M_PLT_XNumber	1	/* 1 reserved PLT entry, PLT[0] */
#define	M_PLT_RESERVSZ	20	/* plt[0] is 5 insns, rather than the usual 3 */

#define	M_GOT_XNumber	3	/* 3 reserved got entries */
#define	M_GOT_XDYNAMIC	0	/* got[0] == _DYNAMIC */
#define	M_GOT_XLINKMAP	1	/* got[1] == link map */
#define	M_GOT_XRTLD	2	/* got[2] == rtbinder */
#define	M_GOT_ENTSIZE	4
#define	M_WORD_ALIGN	4
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_ELF_ARM_H */
