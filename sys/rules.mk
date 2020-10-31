# Copyright (C) 2014 Apple, Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple, Inc.
#
# generic sys stuff
LOCAL_DIR := $(GET_LOCAL_DIR)

MODULES	+= \
	lib/cbuf

# Some of these files contain nothing if the corresponding WITH_XXX is not defined:
#
# menu.o		WITH_MENU
# simple_menu.o		WITH_MENU

ALL_OBJS += \
	$(LOCAL_DIR)/boot.o \
	$(LOCAL_DIR)/callout.o \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/halt.o \
	$(LOCAL_DIR)/hash.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/lock.o \
	$(LOCAL_DIR)/mem.o \
	$(LOCAL_DIR)/menu.o \
	$(LOCAL_DIR)/simple_menu.o \
	$(LOCAL_DIR)/task.o \
	$(LOCAL_DIR)/time.o

# conditionally include the security module
ifeq (${WITH_NO_SECURITY},true)
OPTIONS += \
	WITH_NO_SECURITY=1

else

ALL_OBJS += \
	$(LOCAL_DIR)/security.o
endif
