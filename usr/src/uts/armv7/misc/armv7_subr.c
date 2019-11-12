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
 * Misc. functions that are in ASM on other platforms. Some of these could be
 * from common/util/string.c, but at the moment we need to bootstrap and we
 * don't have all of the requisite headers.
 */

#include <sys/types.h>
#include <sys/null.h>

size_t
strlen(const char *s)
{
	size_t ret = 0;

	for (; *s != '\0'; s++) {
		ret++;
	}

	return (ret);
}

char *
strchr(const char *s, int c)
{
	while (*s != '\0') {
		if (*s == c) {
			return ((char *)s);
		}
		s++;
	}

	return (NULL);
}

/*
 * XXX From common/util/string.c.
 */
int
strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

/*
 * XXX From common/util/string.c.
 */
int
strncmp(const char *s1, const char *s2, size_t n)
{
	if (s1 == s2)
		return (0);
	n++;
	while (--n != 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return ((n == 0) ? 0 : *(unsigned char *)s1 - *(unsigned char *)--s2);
}
