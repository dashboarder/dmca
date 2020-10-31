# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBENV_DIR		:=	$(GET_LOCAL_DIR)
LIBENV_BUILD		:=	$(call TOLIBDIR,$(LIBENV_DIR)/LIBENV.a)
COMMONLIBS		+=	LIBENV

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBENV_OBJS		:=	$(LIBENV_DIR)/env.o

LIBENV_OBJS		:=	$(call TOLIBOBJDIR,$(LIBENV_OBJS))

ALL_DEPS		+=	$(LIBENV_OBJS:%o=%d)

$(LIBENV_BUILD):	$(LIBENV_OBJS)

endif
