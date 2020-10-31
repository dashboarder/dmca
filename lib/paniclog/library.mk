# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBPANICLOG_DIR		:=	$(GET_LOCAL_DIR)
LIBPANICLOG_BUILD	:=	$(call TOLIBDIR,$(LIBPANICLOG_DIR)/LIBPANICLOG.a)
COMMONLIBS		+=	LIBPANICLOG

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBPANICLOG_OBJS	:=	$(LIBPANICLOG_DIR)/paniclog.o

LIBPANICLOG_OBJS	:=	$(call TOLIBOBJDIR,$(LIBPANICLOG_OBJS))

ALL_DEPS		+=	$(LIBPANICLOG_OBJS:%o=%d)

$(LIBPANICLOG_BUILD):	$(LIBPANICLOG_OBJS)

endif
