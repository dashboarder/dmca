# Copyright (C) 2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBDEVICETREE_DIR	:=	$(GET_LOCAL_DIR)
LIBDEVICETREE_BUILD	:=	$(call TOLIBDIR,$(LIBDEVICETREE_DIR)/LIBDEVICETREE.a)
COMMONLIBS		+=	LIBDEVICETREE

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBDEVICETREE_OBJS	:=	$(LIBDEVICETREE_DIR)/devicetree.o	\
				$(LIBDEVICETREE_DIR)/devicetree_load.o	\
				$(LIBDEVICETREE_DIR)/devicetree_debug.o

LIBDEVICETREE_OBJS	:=	$(call TOLIBOBJDIR,$(LIBDEVICETREE_OBJS))

ALL_DEPS		+=	$(LIBDEVICETREE_OBJS:%o=%d)

$(LIBDEVICETREE_BUILD):	$(LIBDEVICETREE_OBJS)

endif
