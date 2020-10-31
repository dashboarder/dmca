# Copyright (C) 2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBLM3534_DIR		:=	$(GET_LOCAL_DIR)
LIBLM3534_BUILD		:=	$(call TOLIBDIR,$(LIBLM3534_DIR)/LIBLM3534.a)
COMMONLIBS		+=	LIBLM3534

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBLM3534_OBJS		:=	$(LIBLM3534_DIR)/lm3534.o

LIBLM3534_OBJS		:=	$(call TOLIBOBJDIR,$(LIBLM3534_OBJS))

ALL_DEPS		+=	$(LIBLM3534_OBJS:%o=%d)

$(LIBLM3534_BUILD):	$(LIBLM3534_OBJS)

endif
