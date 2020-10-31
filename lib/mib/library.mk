# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBMIB_DIR	:=	$(GET_LOCAL_DIR)
LIBMIB_BUILD	:=	$(call TOLIBDIR,$(LIBMIB_DIR)/LIBMIB.a)
COMMONLIBS	+=	LIBMIB

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBMIB_OBJ_DIR	:=	$(call TOLIBOBJDIR,$(LIBMIB_DIR))

MIB_GEN_DEF	:=	$(LIBMIB_DIR)/mib_gen_def.awk
MIB_BASE	:=	$(LIBMIB_DIR)/mib.base

LIBMIB_OBJS	:= \
			$(LIBMIB_OBJ_DIR)/mib.o \
			$(LIBMIB_OBJ_DIR)/mib_nodes.o

$(LIBMIB_BUILD):	$(LIBMIB_OBJS)

$(LIBMIB_OBJ_DIR)/mib_nodes.c:	$(MIB_BASE) $(MIB_GEN_DEF)
	@echo GEN $(LIBMIB_OBJ_DIR)/mib_nodes.c
	$(_v)mkdir -p $(LIBMIB_OBJ_DIR)
	$(_v)awk -f $(MIB_GEN_DEF) $(LIBMIB_OBJ_DIR) $(MIB_BASE)

ALL_DEPS	+=	$(LIBMIB_OBJS:%o=%d)

endif
