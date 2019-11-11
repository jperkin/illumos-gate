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
 * Copyright (c) 1992, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*
 * Copyright (c) 2013 Joyent, Inc.  All rights reserved.
 */

#ifndef _SYS_MACHPARAM_H
#define	_SYS_MACHPARAM_H

#if !defined(_ASM)
#include <sys/types.h>
#endif

/*
 * Machine dependent paramenters and limits, ARM edition
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASM
#define	ADDRESS_C(c)    c ## ul
#else   /* _ASM */
#define	ADDRESS_C(c)    (c)
#endif  /* _ASM */

/*
 * XXX Okay, let's be honest. We haven't even tested more than one CPU at all
 * here, but armv6 can support two CPUs at least. ARMv7 likely can support more.
 * At this point, it's not really clear how the different archs will work in the
 * kernel. Until we figure that out, we'll roll this way.
 */
#define	NCPU		2
#define	NCPU_LOG2	1

/* NCPU_P2 is NCPU rounded to a power of 2 */
#define	NCPU_P2	(1 << NCPU_LOG2)

/*
 * Define the FPU symbol if we could run on a machine with an external
 * FPU (i.e. not integrated with the normal machine state like the vax).
 *
 * The fpu is defined in the architecture manual, and the kernel hides
 * its absence if it is not present, that's pretty integrated, no?
 */

/* supported page sizes */
#define	MMU_PAGE_SIZES	4	/* 4k, 64k, 1M, 16M */

/*
 * MMU_PAGES* describes the physical page size used by the mapping hardware.
 * PAGES* describes the logical page size used by the system.
 */

#define	MMU_PAGESIZE	0x1000		/* 4096 bytes */
#define	MMU_PAGESHIFT	12		/* log2(MMU_PAGESIZE) */
#define	MMU_PAGEOFFSET	(MMU_PAGESIZE-1) /* Mask of address bits in page */
#define	MMU_PAGEMASK	(~MMU_PAGEOFFSET)

#define	MMU_PAGESHIFT64K	16
#define	MMU_PAGESIZE64K		(1 << MMU_PAGESHIFT64K)
#define	MMU_PAGEOFFSET64K	(MMU_PAGESIZE64K - 1)
#define	MMU_PAGEMASK64K		(~MMU_PAGEOFFSET64K)

#define	MMU_PAGESHIFT1M		20
#define	MMU_PAGESIZE1M		(1 << MMU_PAGESHIFT1M)
#define	MMU_PAGEOFFSET1M	(MMU_PAGESIZE1M - 1)
#define	MMU_PAGEMASK1M		(~MMU_PAGEOFFSET1M)

#define	MMU_PAGESHIFT16M	24
#define	MMU_PAGESIZE16M		(1 << MMU_PAGESHIFT16M)
#define	MMU_PAGEOFFSET16M	(MMU_PAGESIZE16M - 1)
#define	MMU_PAGEMASK16M		(~MMU_PAGEOFFSET16M)


#define	PAGESIZE	0x1000		/* All of the above, for logical */
#define	PAGESHIFT	12
#define	PAGEOFFSET	(PAGESIZE - 1)
#define	PAGEMASK	(~PAGEOFFSET)

/*
 * DATA_ALIGN is used to define the alignment of the Unix data segment.
 */
#define	DATA_ALIGN	PAGESIZE

/*
 * DEFAULT KERNEL THREAD stack size (in pages).
 */
#define	DEFAULTSTKSZ_NPGS	3
#define	DEFAULTSTKSZ	(DEFAULTSTKSZ_NPGS * PAGESIZE)

/*
 * KERNELBASE is the virtual address at which the kernel segments start in
 * all contexts.
 *
 * common/conf/param.c requires a compile time defined value for KERNELBASE.
 * This value is save in the variable _kernelbase.  _kernelbase may then be
 * modified with to a different value in i86pc/os/startup.c.
 *
 * Most code should be using kernelbase, which resolves to a reference to
 * _kernelbase.
 */
#ifdef DEBUG
#define	KERNELBASE	ADDRESS_C(0xc8000000)
#else
#define	KERNELBASE	ADDRESS_C(0xd4000000)
#endif

#define	KERNEL_TEXT	ADDRESS_C(0xfe800000)

/*
 * Size of the unmapped "red zone" at the very bottom of the kernel's address
 * space.  Since segmap starts immediately above the red zone, this needs to be
 * MAXBSIZE aligned.
 */
#define	KERNEL_REDZONE_SIZE   MAXBSIZE

/*
 * The heap has a region allocated from it of HEAPTEXT_SIZE bytes specifically
 * for module text.
 */
#define	HEAPTEXT_SIZE		(64 * 1024 * 1024)	/* bytes */

/*
 * ARGSBASE is the base virtual address of the range which the kernel uses to
 * map the arguments for exec. We set this to a value at the high end of the
 * kernel address space in a similar fashion to x86.
 */
#define	ARGSBASE	ADDRESS_C(0xffc00000)

/*
 * Virtual address range available to the debugger
 * We place it just above the kernel text (4M) and kernel data (4M).
 */
#define	SEGDEBUGBASE	(KERNEL_TEXT + ADDRESS_C(0x800000))
#define	SEGDEBUGSIZE	ADDRESS_C(0x400000)

/*
 * Define upper limit on user address space.  Note that we don't need to give
 * ourselves an explicit red zone here as that'll be taken care of as we set up
 * the kernel address space.
 */
#define	USERLIMIT	KERNELBASE
#define	USERLIMIT32	USERLIMIT

/*
 * The exception table is always mapped into the high vector space on illumos.
 */
#define	EXCEPTION_ADDRESS	ADDRESS_C(0xffff0000)

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MACHPARAM_H */
