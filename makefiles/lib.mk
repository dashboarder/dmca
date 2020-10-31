# Copyright (C) 2008-2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

################################################################################
# Makefile to build a library common to multiple build targets in the iBoot tree
#
# We expect the following to be set:
#
# MODULE
#	Path to a directory containing a library.mk Makefile for the library
#	we are going to build.
#
# TAG
#	A unique identifier for the library we are going to build.
#
# OPTIONS
#
#	Options to pass to the compiler when building the library.
#

include makefiles/macros.mk
include makefiles/config.mk

LIBDIR			:=	$(OBJROOT)/build/lib-$(TAG)
LIBOBJDIR		:=	$(addprefix $(LIBDIR)/,$(MODULE))
GLOBAL_ALLFLAGS		+=	$(OPTIONS)
LIBRARY_LDFLAGS		+=	$(OPTIONS)

# XXX HACK ALERT
# It would be nice to have both makefile options and compile options encoded in the
# library spec.  Currently the only one we need is ARCH, so just hack it for now.
ifneq ($(findstring arm64,$(TAG)),)
ARCH			:=	arm64
else
ifneq ($(findstring arm,$(TAG)),)
ARCH			:=	arm
endif
endif

# XXX HACK ALERT
# Libraries don't process options the same way as non-library files. In order
# to get the build flavor in the same format that everything else expects
# we have to play some games.
BUILD			:=	$(lastword $(subst -, ,$(TAG)))
GLOBAL_ALLFLAGS		+=	-D$(BUILD)_BUILD=1

# For ROMRELEASE builds, define RELEASE_BUILD so that everything that uses
# BUILD_RELEASE doesn't have to be touched to use ROMRELEASE also. Also,
# libraries must be completely silent and not emit any debug or panic strings
# for ROMRELEASE builds.
ifeq ($(BUILD),ROMRELEASE)
GLOBAL_ALLFLAGS		+=	-DRELEASE_BUILD=1 -DDEBUG_LEVEL=DEBUG_SILENT -DNO_PANIC_STRINGS=1
endif
ifeq ($(BUILD),RELEASE)
GLOBAL_ALLFLAGS		+=	-DTERSE_PANIC_STRINGS=1
endif

# Read the module makefile
#
# The module makefile defines COMMONLIBS containing the names of libraries
# to build.  For each entry in COMMONLIBS there is:
#
# o an <entry>_BUILD variable which gives the name of the output library.
# o an <entry>_OBJS variable which gives the name of the objects to be built into the library.

include $(MODULE)/library.mk

TARGETVARS		:=	$(addsuffix _BUILD,$(COMMONLIBS))
MAKELIBS		:=	$(foreach i,$(TARGETVARS),$(value $i))

$(MAKELIBS): objs = $(value $(addsuffix _OBJS,$(basename $(notdir $@))))
$(MAKELIBS):
	@$(MKDIR)
	@echo AR  $@
	$(_v)$(AR) -crS $@ $(objs)
	$(_v)$(RANLIB) $@

# Now we can define object build rules
include makefiles/build.mk

build:	$(MAKELIBS)

clean:
	$(_v)rm -rf $(LIBDIR)

# Empty rule for .d files
%.d:
%.Td:

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(ALL_DEPS)
endif

