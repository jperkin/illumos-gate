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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions that the backend is expected to implement.
 */
extern void aboot_backend_init(void);
extern void aboot_backend_putc(int);

#ifdef __cplusplus
}
#endif

#endif /* _ARMV7_ABOOT_ABOOT_H */
