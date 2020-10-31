# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBHFS_DIR		:=	$(GET_LOCAL_DIR)
LIBHFS_BUILD		:=	$(call TOLIBDIR,$(LIBHFS_DIR)/LIBHFS.a)
COMMONLIBS		+=	LIBHFS

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

GLOBAL_INCLUDES		+=	$(SRCROOT)/arch/arm/include

LIBHFS_OBJS		:=	\
				$(LIBHFS_DIR)/cache.o \
				$(LIBHFS_DIR)/hfs.o \
				$(LIBHFS_DIR)/hfs_fs.o

LIBHFS_OBJS		:=	$(call TOLIBOBJDIR,$(LIBHFS_OBJS))

ALL_DEPS		+=	$(LIBHFS_OBJS:%o=%d)

$(LIBHFS_BUILD):	$(LIBHFS_OBJS)

endif
