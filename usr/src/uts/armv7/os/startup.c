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
 */

#include <sys/types.h>
#include <sys/bootconf.h>
#include <sys/obpdefs.h>
#include <sys/promif.h>

/*
 *	32-bit Kernel's Virtual memory layout.
 *		+-----------------------+
 *		|    exception table    |
 * 0xFFFF0000  -|-----------------------|- EXCEPTION_ADDRESS
 *		|                       |
 * 0xFFC00000  -|-----------------------|- ARGSBASE
 *		|   XXX  debugger?      |
 * 0xFF800000  -|-----------------------|- XXX SEGDEBUBBASE?+
 *		|      Kernel Data	|
 * 0xFEC00000  -|-----------------------|
 *              |      Kernel Text	|
 * 0xFE800000  -|-----------------------|- KERNEL_TEXT
 *		|                       |
 *		|    XXX No idea yet    |
 *		|                       |
 * 0xC8002000  -|-----------------------|- XXX segmap_start?
 *		|       Red Zone        |
 * 0xC8000000  -|-----------------------|- kernelbase / userlimit (floating)
 *		|      User Stack       |
 *		|			|
 *		|                       |
 *
 *		:                       :
 *		|    shared objects     |
 *		:                       :
 *
 *		:                       :
 *		|       user data       |
 *             -|-----------------------|-
 *		|       user text       |
 * 0x00002000  -|-----------------------|-  XXX Not necessairily truetoday
 *		|       invalid         |
 * 0x00000000  -|-----------------------|-
 *
 * + Item does not exist at this time.
 */

struct bootops		*bootops = 0;	/* passed in from boot */
struct bootops		**bootopsp;
struct boot_syscalls	*sysp;		/* passed in from boot */

char kern_bootargs[OBP_MAXPATHLEN];
char kern_bootfile[OBP_MAXPATHLEN];

caddr_t s_text;		/* start of kernel text segment */
caddr_t e_text;		/* end of kernel text segment */
caddr_t s_data;		/* start of kernel data segment */
caddr_t e_data;		/* end of kernel data segment */
caddr_t modtext;	/* start of loadable module text reserved */
caddr_t e_modtext;	/* end of loadable module text reserved */
caddr_t moddata;	/* start of loadable module data reserved */
caddr_t e_moddata;	/* end of loadable module data reserved */

/*
 * Some CPUs have holes in the middle of the 64-bit virtual address range.
 */
uintptr_t hole_start, hole_end;

/*
 * PROM debugging facilities
 */
int prom_debug = 1;

/*
 * VM related data
 */
long page_hashsz;		/* Size of page hash table (power of two) */
unsigned int page_hashsz_shift;	/* log2(page_hashsz) */
struct page *pp_base;		/* Base of initial system page struct array */
struct page **page_hash;	/* Page hash table */
pad_mutex_t *pse_mutex;		/* Locks protecting pp->p_selock */
size_t pse_table_size;		/* Number of mutexes in pse_mutex[] */
int pse_shift;			/* log2(pse_table_size) */

/*
 * Cache size information filled in via cpuid.
 */
int l2cache_sz;
int l2cache_linesz ;
int l2cache_assoc;

/*
 * Do basic set up.
 */
static void
startup_init()
{
	if (BOP_GETPROPLEN(bootops, "prom_debug") >= 0) {
		++prom_debug;
		prom_printf("prom_debug found in boot enviroment");
	}
}

static void
startup_memlist()
{
	bop_panic("startup_memlist");
}

static void
startup_kmem()
{
	bop_panic("startup_kmem");
}

static void
startup_vm()
{
	bop_panic("startup_vm");
}

static void
startup_modules()
{
	bop_panic("startup_modules");
}

static void
startup_end()
{
	bop_panic("startup_end");
}

/*
 * Our kernel text is at 0xfe800000, data at 0xfec000000, and exception vector
 * at 0xffff0000. These addresses are all determined and set in stone at link
 * time.
 *
 * This is the ARM machine-dependent startup code. Let's startup the world!
 */
void
startup(void)
{
	startup_init();
	/* TODO if we ever need a board specific startup, it goes here */
	startup_memlist();
	startup_kmem();
	startup_vm();
	startup_modules();
	startup_end();
}
