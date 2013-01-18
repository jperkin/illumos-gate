
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

/* Copyright 2013, Richard Lowe. */

#ifndef _MACHDEP_ARM_H
#define	_MACHDEP_ARM_H

#include <link.h>
#include <sys/machelf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	M_MACH_32	EM_ARM

#ifdef _ELF64
#error "THERE IS NO ARM64"
#else
#define	M_MACH		EM_ARM
#define	M_CLASS		ELFCLASS32
#endif

/* XXXARM This is a SPARC thing, but might end up necessary for v6 v. v7? */
#define	M_MACHPLUS	M_MACH
#define	M_FLAGSPLUS	0

#define	M_DATA		ELFDATA2LSB

#define	M_STRUNC(X)	((X) & ~(M_SEGSIZE - 1))
#define	M_SROUND(X)	(((X) + M_SEGSIZE - 1) & ~(M_SEGSIZE - 1))

/*
 * Relocation type macros.
 */
#define	M_RELOC	Rel

#define	M_SEGM_ALIGN	ELF_ARM_MAXPGSZ

/* XXXARM: These aren't relevant yet, and are guesswork */
#define	M_TLSSTATALIGN	0x8
#define	M_BIND_ADJ	4

#define	M_SEGM_ORIGIN	(Addr)0x08000
#define	M_SEGM_AORIGIN	M_SEGM_ORIGIN

/* Make common relocation information transparent to common code */
#define	M_REL_DT_TYPE	DT_REL
#define	M_REL_DT_SIZE	DT_RELSZ
#define	M_REL_DT_ENT	DT_RELENT
#define	M_REL_DT_COUNT	DT_RELCOUNT
#define	M_REL_SHT_TYPE	SHT_REL
#define	M_REL_ELF_TYPE	ELF_T_REL

#define	M_R_NONE	R_ARM_NONE
#define	M_R_GLOB_DAT	R_ARM_GLOB_DAT
#define	M_R_RELATIVE	R_ARM_RELATIVE
#define	M_R_COPY	R_ARM_COPY
#define	M_R_JMP_SLOT	R_ARM_JUMP_SLOT
#define	M_R_FPTR	R_ARM_NONE
#define	M_R_ARRAYADDR	R_ARM_GLOB_DAT
#define	M_R_NUM		R_ARM_NUM

#define	M_R_REGISTER	M_R_NONE

/* DT_REGISTER is not valid on ARM */
#define	M_DT_REGISTER	0xffffffff

#define	M_PLT_SHF_FLAGS	(SHF_ALLOC | SHF_EXECINSTR)

#define	M_DATASEG_PERM	(PF_R | PF_W)
#define	M_STACK_PERM	(PF_R | PF_W)

/*
 * Define a set of identifies for special sections.  These allow the sections
 * to be ordered within the output file image.  These values should be
 * maintained consistently, where appropriate, in each platform specific header
 * file.
 *
 *  -	null identifies that this section does not need to be added to the
 *	output image (ie. shared object sections or sections we're going to
 *	recreate (sym tables, string tables, relocations, etc.)).
 *
 *  -	any user defined section will be first in the associated segment.
 *
 *  -	interp and capabilities sections are next, as these are accessed
 *	immediately the first page of the image is mapped.
 *
 *  -	objects that do not provide an interp normally have a read-only
 *	.dynamic section that comes next (in this case, there is no need to
 *	update a DT_DEBUG entry at runtime).
 *
 *  -	the syminfo, hash, dynsym, dynstr and rel's are grouped together as
 *	these will all be accessed together by ld.so.1 to perform relocations.
 *
 *  -	the got, dynamic, and plt are grouped together as these may also be
 *	accessed first by ld.so.1 to perform relocations, fill in DT_DEBUG
 *	(executables only), and .plt[0].
 *
 *  -	unknown sections (stabs, comments, etc.) go at the end.
 *
 * Note that .tlsbss/.bss are given the largest identifiers.  This insures that
 * if any unknown sections become associated to the same segment as the .bss,
 * the .bss sections are always the last section in the segment.
 */
#define	M_ID_NULL	0x00
#define	M_ID_USER	0x01

#define	M_ID_INTERP	0x02			/* SHF_ALLOC */
#define	M_ID_CAP	0x03
#define	M_ID_CAPINFO	0x04
#define	M_ID_CAPCHAIN	0x05

#define	M_ID_DYNAMIC	0x06			/* if no .interp, then no */
						/*    DT_DEBUG is required */
#define	M_ID_UNWINDHDR	0x07
#define	M_ID_UNWIND	0x08

#define	M_ID_SYMINFO	0x09
#define	M_ID_HASH	0x0a
#define	M_ID_LDYNSYM	0x0b			/* always right before DYNSYM */
#define	M_ID_DYNSYM	0x0c
#define	M_ID_DYNSTR	0x0d
#define	M_ID_VERSION	0x0e
#define	M_ID_DYNSORT	0x0f
#define	M_ID_REL	0x10
#define	M_ID_ARRAY	0x11
#define	M_ID_TEXT	0x12			/* SHF_ALLOC + SHF_EXECINSTR */
#define	M_ID_DATA	0x20

/*	M_ID_USER	0x01			dual entry - listed above */
#define	M_ID_GOTDATA	0x02			/* SHF_ALLOC + SHF_WRITE */
#define	M_ID_GOT	0x03
#define	M_ID_PLT	0x04
/*	M_ID_DYNAMIC	0x06			dual entry - listed above */
/*	M_ID_UNWIND	0x08			dual entry - listed above */

#define	M_ID_UNKNOWN	0xfc			/* just before TLS */

#define	M_ID_TLS	0xfd			/* just before bss */
#define	M_ID_TLSBSS	0xfe
#define	M_ID_BSS	0xff

#define	M_ID_SYMTAB_NDX	0x02			/* ! SHF_ALLOC */
#define	M_ID_SYMTAB	0x03
#define	M_ID_STRTAB	0x04
#define	M_ID_DYNSYM_NDX	0x05
#define	M_ID_NOTE	0x06

#ifdef __cplusplus
}
#endif

#endif /* _MACHDEP_ARM_H */
