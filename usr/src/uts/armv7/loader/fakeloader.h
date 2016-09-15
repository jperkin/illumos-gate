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
 * Copyright (c) 2015 Josef 'Jeff' Sipek <jeffpc@josefsipek.net>
 */

#ifndef _FAKELOADER_H
#define	_FAKELOADER_H

/*
 * The hacky version of arm uniboot that is exactly for a few systems.
 */

#include <sys/stdint.h>
#include <sys/atag.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fakeloader_hdr {
	unsigned char fh_magic[4];	/* Magic! */
	uint32_t fh_unix_size;		/* How large is unix */
	uint32_t fh_unix_offset;	/* Offset from start to unix */
	uint32_t fh_archive_size;	/* How large is the archive */
	uint32_t fh_archive_offset;	/* Offset from start to archive */
} fakeloader_hdr_t;

#define	FH_MAGIC0	'i'
#define	FH_MAGIC1	'f'
#define	FH_MAGIC2	'b'
#define	FH_MAGIC3	'h'

/*
 * Backend operations, eg. what a given board must implement at the moment
 */
extern void fakeload_backend_init(void);
extern void fakeload_backend_putc(int);
extern void fakeload_backend_addmaps(atag_header_t *);

/*
 * ASM operations
 */
extern void fakeload_leave_hyp(void);
extern void fakeload_unaligned_enable(void);
extern void fakeload_mmu_enable(void);
extern void fakeload_pt_setup(uintptr_t);
extern void fakeload_exec(void *, void *, atag_header_t *, uintptr_t);

extern void armv7_dcache_disable(void);
extern void armv7_dcache_enable(void);
extern void armv7_dcache_inval(void);
extern void armv7_dcache_flush(void);

extern void armv7_icache_disable(void);
extern void armv7_icache_enable(void);
extern void armv7_icache_inval(void);

#ifdef __cplusplus
}
#endif

#endif /* _FAKELOADER_H */
