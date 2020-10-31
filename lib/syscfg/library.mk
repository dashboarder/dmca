# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBSYSCFG_DIR		:=	$(GET_LOCAL_DIR)
LIBSYSCFG_BUILD		:=	$(call TOLIBDIR,$(LIBSYSCFG_DIR)/LIBSYSCFG.a)
COMMONLIBS		+=	LIBSYSCFG

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBSYSCFG_OBJS		:=	$(LIBSYSCFG_DIR)/syscfg.o

LIBSYSCFG_OBJS		:=	$(call TOLIBOBJDIR,$(LIBSYSCFG_OBJS))

ALL_DEPS		+=	$(LIBSYSCFG_OBJS:%o=%d)

$(LIBSYSCFG_BUILD):	$(LIBSYSCFG_OBJS)

endif
