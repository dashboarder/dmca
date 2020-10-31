# Copyright (C) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBCPP_DIR		:=	$(GET_LOCAL_DIR)
LIBCPP_BUILD		:=	$(call TOLIBDIR,$(LIBCPP_DIR)/LIBCPP.a)
COMMONLIBS		+=	LIBCPP

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

# base library files
LIBCPP_OBJS		:=	$(LIBCPP_DIR)/iostream.o \
				$(LIBCPP_DIR)/misc.o \
				$(LIBCPP_DIR)/new.o

# Do not add this until we sort out why we aren't seeing
# static constructors linked into the built object.
#				$(LIBCPP_DIR)/start.o

LIBCPP_OBJS		:=	$(call TOLIBOBJDIR,$(LIBCPP_OBJS))
ALL_DEPS		+=	$(LIBCPP_OBJS:%o=%d)

$(LIBCPP_BUILD):	$(LIBCPP_OBJS)

endif
