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

	.file	"qvirt_boot.s"

/*
 * This file implements the bootstrap for QEMU virtual machine board to get us
 * into aboot.
 *
 * A few notes about the memory layout. We're told that RAM starts at 1 GiB on
 * this system, aka 0x40000000. From there, here's how we lay things out:
 *
 * +------------+
 * | PTE Root   |
 * +------------+
 * | Boot Arch. | 
 * +------------+ SW Address (Address set by firmware)
 *      ...
 * +------------+ 0x41000000+...
 * | aboot ELF  |
 * +------------+ 0x41000000
 * | aboot stack|
 * +------------+ 0x40900000
 * | aboot data |
 * +------------+ 0x40800000 (Limit, probably less)
 * |     FDT    |
 * +------------+ 0x40000000
 *
 * Our allocated data that we pass to the kernel, along with the original FDT is
 * at the start of memory. aboot then works down from there with its stack,
 * hopefully the two not meeting, while the aboot ELF image grows up.
 */

/*
 * This is the address that the flattened device tree starts at for the qemu
 * virtual board.
 */
#define	QVIRT_FDT	0x40000000

/*
 * We need to get a bootstrap stack pointer. For this we rely on the memory
 * layout that we have chosen.
 */
#define	QVIRT_SP	0x41000000

#include <sys/asm_linkage.h>

	ENTRY(_start)
	mov	r0, #0
	mov	r1, #0
	mvn	r1, r1
	mov	r2, #QVIRT_FDT
	mov	sp, #QVIRT_SP
	bl	aboot_main
	SET_SIZE(_start)
