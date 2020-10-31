# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBPARTITION_DIR	:=	$(GET_LOCAL_DIR)
LIBPARTITION_BUILD	:=	$(call TOLIBDIR,$(LIBPARTITION_DIR)/LIBPARTITION.a)
COMMONLIBS		+=	LIBPARTITION

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBPARTITION_OBJS	:=	\
				$(LIBPARTITION_DIR)/gpt.o \
				$(LIBPARTITION_DIR)/lwvm.o \
				$(LIBPARTITION_DIR)/mbr.o \
				$(LIBPARTITION_DIR)/partition.o

LIBPARTITION_OBJS	:=	$(call TOLIBOBJDIR,$(LIBPARTITION_OBJS))

ALL_DEPS		+=	$(LIBPARTITION_OBJS:%o=%d)

$(LIBPARTITION_BUILD):	$(LIBPARTITION_OBJS)

endif
