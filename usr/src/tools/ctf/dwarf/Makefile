#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include		../../Makefile.tools

SUBDIRS	= $(NATIVE_MACH)

all	:= TARGET= all
install	:= TARGET= install
clean	:= TARGET= clean
clobber	:= TARGET= clobber
lint	:= TARGET= lint

.KEEP_STATE:

install all clean clobber: $(SUBDIRS)

lint:

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

FRC:
