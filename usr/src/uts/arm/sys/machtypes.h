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
 * Copyright 2013 (c) Joyent, Inc.  All rights reserved.
 */

#ifndef _SYS_MACHTYPES_H
#define	_SYS_MACHTYPES_H

/*
 * Machine dependent types:
 *
 * 	ARM version
 */

#ifdef __cplusplus
extern "C" {
#endif

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || \
	defined(__EXTENSIONS__)

#define	REG_LABEL_PC	0
#define	REG_LABEL_SP	1
#define	REG_LABEL_R4	2
#define	REG_LABEL_R5	3
#define	REG_LABEL_R6	4
#define	REG_LABEL_R7	5
#define	REG_LABEL_R8	6
#define	REG_LABEL_R9	7
#define	REG_LABEL_R10	8
#define	REG_LABEL_R11	9
#define	REG_LABEL_MAX	10

typedef	struct _label_t { long val[REG_LABEL_MAX]; } label_t;

#endif /* !defined(_POSIX_C_SOURCE)... */

typedef	unsigned char	lock_t;		/* lock work for busy wait */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MACHTYPES_H */
