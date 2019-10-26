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

/*
 * Misc. functions that are in ASM on other platforms.
 */

#include <sys/types.h>

size_t
strlen(const char *s)
{
	size_t ret = 0;

	for (; *s != '\0'; s++) {
		ret++;
	}

	return (ret);
}
