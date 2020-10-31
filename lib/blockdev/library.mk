# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBBLOCKDEV_DIR		:=	$(GET_LOCAL_DIR)
LIBBLOCKDEV_BUILD	:=	$(call TOLIBDIR,$(LIBBLOCKDEV_DIR)/LIBBLOCKDEV.a)
COMMONLIBS		+=	LIBBLOCKDEV

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBBLOCKDEV_OBJS	:=	\
				$(LIBBLOCKDEV_DIR)/blockdev.o \
				$(LIBBLOCKDEV_DIR)/subdev.o \
				$(LIBBLOCKDEV_DIR)/mem_blockdev.o \
				$(LIBBLOCKDEV_DIR)/debug.o

LIBBLOCKDEV_OBJS	:=	$(call TOLIBOBJDIR,$(LIBBLOCKDEV_OBJS))

ALL_DEPS		+=	$(LIBBLOCKDEV_OBJS:%o=%d)

$(LIBBLOCKDEV_BUILD):	$(LIBBLOCKDEV_OBJS)

endif
