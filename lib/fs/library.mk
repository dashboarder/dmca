# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBFS_DIR		:=	$(GET_LOCAL_DIR)
LIBFS_BUILD		:=	$(call TOLIBDIR,$(LIBFS_DIR)/LIBFS.a)
COMMONLIBS		+=	LIBFS

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBFS_OBJS		:=	\
				$(LIBFS_DIR)/fs.o \
				$(LIBFS_DIR)/debug.o

LIBFS_OBJS		:=	$(call TOLIBOBJDIR,$(LIBFS_OBJS))

ALL_DEPS		+=	$(LIBFS_OBJS:%o=%d)

$(LIBFS_BUILD):	$(LIBFS_OBJS)

endif
