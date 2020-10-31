# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBPGINDEP_DIR	:=	$(GET_LOCAL_DIR)
LIBPGINDEP_BUILD:=	$(call TOLIBDIR,$(LIBPGINDEP_DIR)/LIBPGINDEP.a)
COMMONLIBS	+=	LIBPGINDEP

# Force this library's symbols to be imported so that they can't be
# overridden. This library isn't designed with one function per object file
# to allow selective overriding
GLOBAL_LDFORCELIBS	+=	$(LIBPGINDEP_BUILD)

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBPGINDEP_OBJS	:= \
	$(LIBPGINDEP_DIR)/platform_indep.o \
	$(LIBPGINDEP_DIR)/target_indep.o \
	$(LIBPGINDEP_DIR)/target_pass_boot_manifest.o \
	$(LIBPGINDEP_DIR)/breadcrumbs.o

LIBPGINDEP_OBJS	:=	$(call TOLIBOBJDIR,$(LIBPGINDEP_OBJS))

$(LIBPGINDEP_BUILD):	$(LIBPGINDEP_OBJS)

ALL_DEPS	+=	$(LIBPGINDEP_OBJS:%o=%d)

endif
