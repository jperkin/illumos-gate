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

UTSBASE = ..

include Makefile.armv7

all			:=	TARGET= all
def			:=	TARGET= def
check			:=	TARGET= check
clean			:=	TARGET= clean
clobber			:=	TARGET= clobber
install_h		:=	TARGET= install_h
modlist			:=	TARGETf= modlist
modlist	modlist.arm	:=	NO_STATE= -K $$MODSTATE$$$$

.KEEP_STATE:

.PARALLEL: $(PARALLEL_KMODS) $(XMODS) modlist modlist.arm

INITIAL_TARGETS = \
	# genassym XXX Once we're good
	# unix XXX Once we're good

#XXX: Compare these to i86pc to see how they should look once fleshed out
all def clean clobber: genassym .WAIT $(KMODS) $(XMODS) $(IMPLEMENTATIONS)
install: install_platforms genassym .WAIT $(KMODS) $(XMODS) $(IMPLEMENTATIONS)

# list the modules under armv6.
modlist: $(KMODS) $(XMODS) $(IMPLEMENTATIONS)

# list the modules for Install -k armv7.
modlist.karch: modlist modlist.arm

modlist.arm:
	@cd $(SRC)/uts/arm; pwd; $(MAKE) $(NO_STATE) modlist

#
# XXX No genassym yet
#
genassym: FRC
	#@cd $@; pwd; $(MAKE) $(NO_STATE) $(TARGET)

setup: FRC

$(IMPLEMENTATIONS):	FRC
	@cd $@; pwd; THISIMPL=$@ $(MAKE) $(NO_STATE) $(TARGET)

$(XMODS):	FRC
	@cd $@; pwd; $(MAKE) $(NO_STATE) $(TARGET); \


install_h check: $(IMPLEMENTATIONS) FRC
	@cd sys; pwd; $(MAKE) $(TARGET)

PLAT_LINKS =

install_platforms: $(ROOT_PSM_DIR) $(USR_PSM_DIR) \
	$(ROOT_PLAT_LINKS) $(USR_PLAT_LINKS) \
	$(OEM_USR_PLAT_LINKS)

include ../Makefile.targ

XRDIRS = ../armv7 ../arm ../common

XRPRUNE = sun4u sun4v sun4 sun sffmu i86pc i86xpv intel sparc

cscope.out tags: FRC
	$(XREF) -x $@
