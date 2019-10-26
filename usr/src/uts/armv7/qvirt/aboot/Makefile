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
#
#	Path to the base of the uts directory tree (usually /usr/src/uts).
#
UTSBASE	= ../../..
#
#	Define the module and object file sets.
#
LOADER		= aboot
LOADER_BIN	= $(OBJS_DIR)/$(LOADER)
ROOTMODULE	= $(ROOT_QVPB_KERN_DIR)/$(LOADER)
OBJECTS		= $(ABOOT_FILES:%=$(OBJS_DIR)/%)

#
#	Include common rules.
#
include $(UTSBASE)/armv7/qvirt/Makefile.qvirt

#
#	Define targets
#
ALL_TARGET	= $(LOADER_BIN)
INSTALL_TARGET	= $(LOADER_BIN) $(ROOTMODULE)

#
#	Overrides
#
CLEANFILES	+= $(OBJECTS)

CLOBBERFILES	= $(CLEANFILES) $(LOADER_BIN)

CFLAGS 		+= $(CCVERBOSE) \
		-DELF_TARGET_ARM

#
# XXX We're apparently getting an unsupported armv7 relocation. Use tihs
# to get us over the hump with gcc7 as armv6 doesn't seem to cause it
# for reasons unknown.
#
CFLAGS		+= -_gcc=-march=armv6

MAPFILE		= $(UTSBASE)/armv7/qvirt/conf/Mapfile.loader

#
#	Default build targets.
#
.KEEP_STATE:

def:		$(DEF_DEPS)

all:		$(ALL_DEPS)

clean:		$(CLEAN_DEPS)

clobber:	$(CLOBBER_DEPS)

install:  	$(INSTALL_DEPS)

symcheck:       $(SYM_DEPS)

$(LOADER_BIN): $(OBJECTS)
	$(LD) -dy -b -znointerp -o $@ -e _start -M $(MAPFILE) \
	$(OBJECTS)
	$(CTFMERGE) $(CTFMRGFLAGS) -L VERSION -o $(LOADER_BIN) $(OBJECTS)

#
#	Include common targets.
#
include $(UTSBASE)/armv7/qvirt/Makefile.targ
