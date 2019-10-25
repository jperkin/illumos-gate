#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#
# Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.
#
# Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
# Use is subject to license terms.
#

LIBCBASE=	.
LIBCDIR=	$(SRC)/lib/libc
LIBRARY=	libc.a
LIB_PIC=	libc_pic.a
VERS=		.1
CPP=		/usr/lib/cpp
TARGET_ARCH=	arm

# extra files for the clean target
CLEANFILES=			\
	assym.h			\
	genassym

CLOBBERFILES +=	$(LIB_PIC)

include ../../Makefile.lib

.KEEP_STATE:

# include common libc targets
include ../Makefile.targ

# assym.h build rules

assym.h: FRC
	echo "#error If you need me, something is wrong" >> $@

FRC:
