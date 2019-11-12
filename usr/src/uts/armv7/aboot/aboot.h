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

#ifndef _ARMV7_ABOOT_ABOOT_H
#define	_ARMV7_ABOOT_ABOOT_H

/*
 * Common definitions used for aboot.
 */

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aboot_info {
	uintptr_t	abi_ptarena;
	uintptr_t	abi_fdt;
	uintptr_t	abi_freemem;
	size_t		abi_freelen;
	uintptr_t	abi_aboot;
	size_t		abi_abootlen;
	uintptr_t	abi_cons;
	uintptr_t	abi_conslen;
} aboot_info_t;

/*
 * Functions that the backend is expected to implement.
 */
extern void aboot_backend_init(void);
extern void aboot_backend_putc(int);
extern void aboot_map_console(void);

/*
 * Functions the backends can call.
 */

typedef enum aboot_prot {
	ABOOT_P_R	= 1 << 0,
	ABOOT_P_W	= 1 << 1,
	ABOOT_P_X	= 1 << 2,
	ABOOT_P_DEV	= 1 << 3
} aboot_prot_t;

extern void aboot_panic(const char *);
extern void aboot_map_1mb(uintptr_t, uintptr_t, aboot_prot_t);
extern aboot_info_t *aboot_infop;

extern void aboot_unaligned_enable(void);
extern void aboot_mmu_program(uintptr_t);
extern void aboot_mmu_enable(void);


#ifdef __cplusplus
}
#endif

#endif /* _ARMV7_ABOOT_ABOOT_H */
