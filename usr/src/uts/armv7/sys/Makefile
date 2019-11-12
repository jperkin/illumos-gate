#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright 2019 Team Twist My ARM
#

include ../../../Makefile.master

HDRS = \
	armv7_cpu.h	\
	machparam.h

ROOTDIR=	$(ROOT)/usr/include/sys
ROOTHDRS=	$(HDRS:%=$(ROOTDIR)/%)
CHECKHDR=	$(HDRS:%.h=%.check)

$(ROOTDIR):
	$(INS.dir)

$(ROOTDIR)/%: %
	$(INS.file)

install_h: $(ROOTDIR) .WAIT $(ROOTHDRS)

check:	$(CHECKHDR)
