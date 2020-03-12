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
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

PROG	= cw.$(TARG_MACH)

SRCS	= ../cw.c

include ../../Makefile.tools

# Bootstrap problem -- we have to build cw before we can use it
NATIVECC = $($(NATIVE_MACH)_CC)

CFLAGS += $(CCVERBOSE)

# Override CFLAGS.  This is needed only for bootstrap of cw.
$(__GNUC)CFLAGS=	-O -D__sun -Wall -Wno-unknown-pragmas -Werror \
			-std=gnu99 -nodefaultlibs
$(__SUNC)CFLAGS=	-xspace -Xa -xildoff -errtags=yes -errwarn=%all \
			-xc99=%all -W0,-xglobalstatic -v


$(__GNUC)LDLIBS +=	-lc
$(__GNUC)LDFLAGS=	$(MAPFILE.NES:%=-Wl,-M%)
$(__GNUC)LDCHECKS=

CSTD=	$(CSTD_GNU99)

CPPFLAGS += -DCW_TARGET_$(TARG_MACH)=1
CPPFLAGS += -DCW_TARGET='"$(TARG_MACH)"'

# Assume we don't have the install.bin available yet
INS.file=       $(RM) $@; $(CP) $< $(@D); $(CHMOD) $(FILEMODE) $@

.KEEP_STATE:

all: $(PROG)

install: all .WAIT $(ROOTONBLDMACHPROG)

lint: lint_SRCS

clean:

$(PROG): $(SRCS)
	$(LINK.c) -o $@ $(SRCS) $(LDLIBS)
	$(POST_PROCESS)

include ../../Makefile.targ
