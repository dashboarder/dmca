# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBPAINT_DIR		:=	$(GET_LOCAL_DIR)
LIBPAINT_BUILD		:=	$(call TOLIBDIR,$(LIBPAINT_DIR)/LIBPAINT.a)
COMMONLIBS		+=	LIBPAINT

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBPAINT_OBJS		:=	$(LIBPAINT_DIR)/paint.o

LIBPAINT_OBJS		:=	$(call TOLIBOBJDIR,$(LIBPAINT_OBJS))

ALL_DEPS		+=	$(LIBPAINT_OBJS:%o=%d)

$(LIBPAINT_BUILD):	$(LIBPAINT_OBJS)

endif
