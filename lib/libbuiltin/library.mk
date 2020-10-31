# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBBUILTIN_DIR		:= $(GET_LOCAL_DIR)
LIBBUILTIN_BUILD	:= $(call TOLIBDIR,$(LIBBUILTIN_DIR)/LIBBUILTIN.a)
COMMONLIBS		+= LIBBUILTIN

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

# base library files
LIBBUILTIN_OBJS :=

# handle architecture-specific overrides for gcc builtin functions
ARCH_BUILTINOPS :=
-include $(LIBBUILTIN_DIR)/$(ARCH)/rules.mk

BUILTINOPS := builtin_divide

# filter out the strops that the arch code doesn't already specify
BUILTINOPS := $(filter-out $(ARCH_BUILTINOPS),$(BUILTINOPS))
BUILTINOPS_FILES := $(addsuffix .o,$(addprefix $(LIBBUILTIN_DIR)/,$(BUILTINOPS)))

LIBBUILTIN_OBJS += \
	$(BUILTINOPS_FILES)

LIBBUILTIN_OBJS := $(call TOLIBOBJDIR,$(LIBBUILTIN_OBJS))

$(LIBBUILTIN_BUILD):	$(LIBBUILTIN_OBJS)

ALL_DEPS += $(LIBBUILTIN_OBJS:%o=%d)

endif
