# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBRAMDISK_DIR		:=	$(GET_LOCAL_DIR)
LIBRAMDISK_BUILD	:=	$(call TOLIBDIR,$(LIBRAMDISK_DIR)/LIBRAMDISK.a)
COMMONLIBS		+=	LIBRAMDISK

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBRAMDISK_OBJS		:=	$(LIBRAMDISK_DIR)/ramdisk.o

LIBRAMDISK_OBJS		:=	$(call TOLIBOBJDIR,$(LIBRAMDISK_OBJS))

ALL_DEPS		+=	$(LIBRAMDISK_OBJS:%o=%d)

$(LIBRAMDISK_BUILD):	$(LIBRAMDISK_OBJS)

endif
