# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBNVRAM_DIR		:=	$(GET_LOCAL_DIR)
LIBNVRAM_BUILD		:=	$(call TOLIBDIR,$(LIBNVRAM_DIR)/LIBNVRAM.a)
COMMONLIBS		+=	LIBNVRAM

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBNVRAM_OBJS		:=	$(LIBNVRAM_DIR)/nvram.o

LIBNVRAM_OBJS		:=	$(call TOLIBOBJDIR,$(LIBNVRAM_OBJS))

ALL_DEPS		+=	$(LIBNVRAM_OBJS:%o=%d)

$(LIBNVRAM_BUILD):	$(LIBNVRAM_OBJS)

endif
