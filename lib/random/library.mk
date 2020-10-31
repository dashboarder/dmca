# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBRANDOM_DIR		:=	$(GET_LOCAL_DIR)
LIBRANDOM_BUILD		:=	$(call TOLIBDIR,$(LIBRANDOM_DIR)/LIBRANDOM.a)
COMMONLIBS		+=	LIBRANDOM

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBRANDOM_OBJS		:=	$(LIBRANDOM_DIR)/random.o

LIBRANDOM_OBJS		:=	$(call TOLIBOBJDIR,$(LIBRANDOM_OBJS))

ALL_DEPS		+=	$(LIBRANDOM_OBJS:%o=%d)

$(LIBRANDOM_BUILD):	$(LIBRANDOM_OBJS)

endif
