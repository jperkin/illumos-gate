/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 2013, Joyent, Inc.  All rights reserved.
 */

#ifndef _SYS_ASM_LINKAGE_H
#define	_SYS_ASM_LINKAGE_H

/*
 * The arm version of this file has been adapated from sparc and intel versions.
 */

/*
 * XXX These files always include <sys/stack.h> and <sys/trap.h>. Because those
 * don't exist yet, we'll just be sad about that, and bypass them for now.
 * Though these should be some of the pieces that we do at some point.
 */
#if 0
#include <sys/trap.h>
#include <sys/stack.h>
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef _ASM	/* The remainder of this file is only for assembly files */

#if !defined(__GNUC_AS__)
#error "unsupported ARM assembler, add support to uts/arm/sys/asm_linkage.h"
#endif
/*
 * These constants can be used to compute offsets into pointer arrays.
 */
/* XXX arm64 */
#define	CPTRSHIFT	2
#define	CLONGSHIFT	2

#define	CPTRSIZE	(1<<CPTRSHIFT)
#define	CLONGSIZE	(1<<CLONGSHIFT)
#define	CPTRMASK	(CPTRSIZE - 1)
#define	CLONGMASK	(CLONGSIZE - 1)


/*
 * profiling causes defintions of the MCOUNT and RTMCOUNT
 * particular to the type
 */
#if defined(PROF) || defined(GPROF)
#error "prof and gprof asm macros are not currently supported on arm"
#endif /* defined(PROF) || defined(GRPOF) */

/*
 * if we are not profiling, MCOUNT should be defined to nothing
 */
#if !defined(PROF) && !defined(GPROF)
#define	MCOUNT(x)
#endif /* !defined(PROF) && !defined(GPROF) */

#define	RTMCOUNT(x)	MCOUNT(x)

/*
 * Macro to define weak symbol aliases. These are similar to the ANSI-C
 *	#pragma weak _name = name
 * except a compiler can determine type. The assembler must be told. Hence,
 * the second parameter must be the type of the symbol (i.e.: function,...)
 */
#define	ANSI_PRAGMA_WEAK(sym, stype)	\
/* CSTYLED */ \
	.weak	_/**/sym; \
/* CSTYLED */ \
	.type	_/**/sym, %stype; \
/* CSTYLED */ \
_/**/sym = sym

/*
 * Like ANSI_PRAGMA_WEAK(), but for unrelated names, as in:
 *	#pragma weak sym1 = sym2
 */
#define	ANSI_PRAGMA_WEAK2(sym1, sym2, stype)	\
	.weak	sym1; \
	.type sym1, %stype; \
sym1	= sym2


/*
 * ENTRY provides the standard procedure entry code and an easy way to
 * insert the calls to mcount for profiling. ENTRY_NP is identical, but
 * never calls mcount.
 */
#define	ENTRY(x) \
	.text; \
	.align	4; \
	.globl	x; \
	.type	x, %function; \
x:	MCOUNT(x)

#define	ENTRY_NP(x) \
	.text; \
	.align	4; \
	.globl	x; \
	.type	x, %function; \
x:	MCOUNT(x)

#define	RTENTRY(x) \
	.text; \
	.align	4; \
	.globl	x; \
	.type	x, %function; \
x:	RTMCOUNT(x)

/*
 * ENTRY2 is identical to ENTRY but provides two labels for the entry point.
 */
#define	ENTRY2(x, y) \
	.text; \
	.align	4; \
	.globl	x; \
	.type	x, %function; \
/* CSTYLED */ \
x:	; \
y:	MCOUNT(x)

#define	ENTRY_NP2(x, y) \
	.text; \
	.align	4; \
	.globl	x; \
	.type	x, %function; \
/* CSTYLED */ \
x:	; \
y:

/*
 * ALTENTRY provides for additional entry points.
 */
#define	ALTENTRY(x) \
	.globl	x; \
	.type	x, %function; \
x:

/*
 * DGDEF and DGDEF2 provide global data declarations.
 *
 * DGDEF provides a word aligned word of storage.
 *
 * DGDEF2 allocates "sz" bytes of storage with **NO** alignment.  This
 * implies this macro is best used for byte arrays.
 *
 * DGDEF3 allocates "sz" bytes of storage with "algn" alignment.
 */
#define	DGDEF2(name, sz) \
	.data; \
	.glbol	name; \
	.type	name, %object; \
	.size	name, sz; \
name:

#define	DGDEF3(name, sz, algn) \
	.data; \
	.align	algn; \
	.globl	name; \
	.type	name, %object; \
	.size	name, sz; \
name:

#define	DGDEF(name)	DGDEF3(name, 4, 4)

/*
 * SET_SIZE trails a function and set the size for the ELF symbol table.
 */
#define	SET_SIZE(x) \
	.size	x, (.-x)

#endif /* _ASM */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_ASM_LINKAGE_H */
